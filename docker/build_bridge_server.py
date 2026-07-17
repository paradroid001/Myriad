#!/usr/bin/env python3
import argparse
import json
import os
import socketserver
import subprocess
import sys
import threading
import time
from datetime import datetime
from pathlib import Path


WATCHED_SUFFIXES = {
    ".c",
    ".cc",
    ".cpp",
    ".cxx",
    ".h",
    ".hh",
    ".hpp",
    ".hxx",
    ".inl",
    ".ixx",
    ".cmake",
    ".txt",
}
WATCHED_FILENAMES = {
    "CMakeLists.txt",
    "CMakePresets.json",
    "CMakeKits.json",
}
MAX_CHANGED_FILES_PREVIEW = 12


class BridgeTrafficLogger:
    def __init__(self, project_root: Path):
        log_dir = project_root / "runtime_logs"
        log_dir.mkdir(parents=True, exist_ok=True)
        timestamp = datetime.now().strftime("%Y%m%d-%H%M%S")
        self.path = log_dir / f"build-bridge-{timestamp}.log"
        self._lock = threading.Lock()

    def write(self, channel: str, message: str) -> None:
        line = f"{datetime.now().isoformat(timespec='milliseconds')} [{channel}] {message}\n"
        with self._lock:
            with self.path.open("a", encoding="utf-8") as handle:
                handle.write(line)

    def comm_rx(self, message: str) -> None:
        self.write("COMM RX", message)

    def comm_tx(self, message: str) -> None:
        self.write("COMM TX", message)

    def build_output(self, message: str) -> None:
        self.write("BUILD", message)


def trim_relative_path(path_text: str) -> Path:
    path = Path(path_text)
    if path.is_absolute():
        raise ValueError("buildDir must be relative to the project root")

    normalized = path.parts
    if any(part == ".." for part in normalized):
        raise ValueError("buildDir must stay inside the project root")

    return path


def load_preset_map(project_root: Path) -> dict[str, dict]:
    kits_path = project_root / "CMakeKits.json"
    with kits_path.open("r", encoding="utf-8") as handle:
        data = json.load(handle)

    if not isinstance(data, list):
        raise ValueError("CMakeKits.json must contain an array of presets")

    return {item["name"]: item for item in data if isinstance(item, dict) and "name" in item}


def resolve_preset_environment(preset: dict) -> dict[str, str]:
    environment = os.environ.copy()
    preset_environment = preset.get("environmentVariables", {})
    if isinstance(preset_environment, dict):
        for key, value in preset_environment.items():
            environment[str(key)] = str(value)

    return environment


def write_bridge_line(handle, lock: threading.Lock, line: str) -> None:
    payload = (line + "\n").encode("utf-8")
    with lock:
        handle.write(payload)
        handle.flush()


def stream_process_output(process: subprocess.Popen, stream_name: str, emit_line, stream_closed: threading.Event) -> None:
    stream = getattr(process, stream_name)
    if stream is None:
        return

    try:
        for raw_line in iter(stream.readline, ""):
            if stream_closed.is_set():
                break

            line = raw_line.rstrip("\r\n")
            if not emit_line(f"@@MYRIAD-BRIDGE-LOG@@ [{stream_name}] {line}"):
                stream_closed.set()
                break
    finally:
        stream.close()


def run_logged_command(command: list[str], cwd: Path, environment: dict[str, str], emit_line, label: str) -> int:
    emit_line(f"@@MYRIAD-BRIDGE-LOG@@ [{label}] {' '.join(command)}")
    process = subprocess.Popen(
        command,
        cwd=cwd,
        env=environment,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        bufsize=1,
    )

    stream_closed = threading.Event()
    threads = []
    for stream_name in ("stdout", "stderr"):
        thread = threading.Thread(target=stream_process_output, args=(process, stream_name, emit_line, stream_closed), daemon=True)
        thread.start()
        threads.append(thread)

    returncode = process.wait()
    for thread in threads:
        thread.join()

    return returncode


def resolve_toolchain_path(project_root: Path, toolchain_text: str) -> Path | None:
    if not toolchain_text:
        return None

    toolchain_path = Path(toolchain_text)
    if not toolchain_path.is_absolute():
        toolchain_path = (project_root / toolchain_path).resolve()

    if not toolchain_path.exists():
        return None

    return toolchain_path


def find_executable(build_dir: Path) -> Path | None:
    target_names = {"testecs", "testecs.exe"}
    search_roots = [
        build_dir / "Examples" / "TestECS",
        build_dir / "Editor",
        build_dir / "bin",
        build_dir,
    ]

    for root in search_roots:
        if not root.exists():
            continue

        for candidate in root.rglob("*"):
            if not candidate.is_file():
                continue

            if candidate.name.lower() in target_names:
                return candidate

    return None


def normalize_target_name(target: str) -> str:
    normalized = target.strip().lower()
    return normalized if normalized else "testecs"


def iter_watch_roots(project_root: Path, normalized_target: str) -> list[Path]:
    roots = [
        project_root / "Engine",
        project_root / "shared",
    ]

    if normalized_target == "testecs":
        roots.append(project_root / "Examples" / "TestECS")
    else:
        roots.append(project_root / "Examples" / normalized_target)

    return roots


def should_watch_file(file_path: Path) -> bool:
    if file_path.name in WATCHED_FILENAMES:
        return True

    return file_path.suffix.lower() in WATCHED_SUFFIXES


def append_changed_file(changed_files_preview: list[str], project_root: Path, candidate: Path) -> None:
    if len(changed_files_preview) >= MAX_CHANGED_FILES_PREVIEW:
        return

    try:
        preview_path = candidate.relative_to(project_root).as_posix()
    except ValueError:
        preview_path = candidate.as_posix()

    changed_files_preview.append(preview_path)


def scan_rebuild_state(project_root: Path, normalized_target: str, last_success_build_time: float) -> tuple[bool, int, list[str]]:
    changed_file_count = 0
    changed_files_preview: list[str] = []

    for root in iter_watch_roots(project_root, normalized_target):
        if not root.exists():
            continue

        for candidate in root.rglob("*"):
            if not candidate.is_file() or not should_watch_file(candidate):
                continue

            if last_success_build_time <= 0.0:
                changed_file_count += 1
                append_changed_file(changed_files_preview, project_root, candidate)
                continue

            try:
                if candidate.stat().st_mtime > last_success_build_time:
                    changed_file_count += 1
                    append_changed_file(changed_files_preview, project_root, candidate)
            except OSError:
                continue

    project_files = [
        project_root / "CMakeLists.txt",
        project_root / "CMakeKits.json",
    ]
    for candidate in project_files:
        if not candidate.exists() or not should_watch_file(candidate):
            continue

        if last_success_build_time <= 0.0:
            changed_file_count += 1
            append_changed_file(changed_files_preview, project_root, candidate)
            continue

        try:
            if candidate.stat().st_mtime > last_success_build_time:
                changed_file_count += 1
                append_changed_file(changed_files_preview, project_root, candidate)
        except OSError:
            continue

    rebuild_needed = changed_file_count > 0
    return rebuild_needed, changed_file_count, changed_files_preview


def get_target_state(server: "BuildBridgeServer", normalized_target: str) -> dict:
    with server.state_lock:
        state = server.build_state.get(normalized_target)
        if state is None:
            state = {"lastSuccessBuildTime": 0.0}
            server.build_state[normalized_target] = state
        return dict(state)


def set_target_last_success(server: "BuildBridgeServer", normalized_target: str, success_time: float) -> None:
    with server.state_lock:
        state = server.build_state.get(normalized_target)
        if state is None:
            state = {"lastSuccessBuildTime": 0.0}
            server.build_state[normalized_target] = state
        state["lastSuccessBuildTime"] = success_time


def make_status_response(project_root: Path, presets: dict[str, dict], normalized_target: str, last_success_build_time: float) -> dict:
    rebuild_needed, changed_file_count, changed_files_preview = scan_rebuild_state(project_root, normalized_target, last_success_build_time)
    return {
        "success": True,
        "status": "Bridge reachable.",
        "bridge": "online",
        "projectRoot": project_root.as_posix(),
        "presetCount": len(presets),
        "target": normalized_target,
        "rebuildNeeded": rebuild_needed,
        "changedFileCount": changed_file_count,
        "changedFilesPreview": changed_files_preview,
    }


def run_build(server: "BuildBridgeServer", project_root: Path, presets: dict[str, dict], request: dict, emit_line) -> dict:
    normalized_target = normalize_target_name(str(request.get("target", "TestECS")))
    target_state = get_target_state(server, normalized_target)
    last_success_build_time = float(target_state.get("lastSuccessBuildTime", 0.0))

    action = str(request.get("action", "")).strip().lower()
    if action == "status":
        return make_status_response(project_root, presets, normalized_target, last_success_build_time)

    preset_name = str(request.get("preset", "")).strip()
    if not preset_name:
        return {"success": False, "status": "Missing preset name."}

    preset = presets.get(preset_name)
    if preset is None:
        return {"success": False, "status": f"Unknown preset: {preset_name}"}

    environment = resolve_preset_environment(preset)

    build_dir_text = str(request.get("buildDir", "")).strip()
    if not build_dir_text:
        build_dir = project_root / "build" / preset_name / "Debug"
        build_dir_relative = Path("build") / preset_name / "Debug"
    else:
        build_dir_relative = trim_relative_path(build_dir_text)
        build_dir = (project_root / build_dir_relative).resolve()

    target = str(request.get("target", "TestECS")).strip() or "TestECS"
    rebuild_needed_before_build, changed_file_count_before_build, changed_files_preview_before_build = scan_rebuild_state(project_root, normalized_target, last_success_build_time)
    build_dir.mkdir(parents=True, exist_ok=True)

    configure_command = ["cmake", "-S", str(project_root), "-B", str(build_dir), "-DCMAKE_BUILD_TYPE=Debug"]
    toolchain_path = resolve_toolchain_path(project_root, str(preset.get("toolchainFile", "")))
    if toolchain_path is not None:
        configure_command.append(f"-DCMAKE_TOOLCHAIN_FILE={toolchain_path}")

    configure_returncode = run_logged_command(configure_command, project_root, environment, emit_line, "configure")
    if configure_returncode != 0:
        return {"success": False, "status": "Configure failed."}

    build_command = ["cmake", "--build", str(build_dir), "--target", target]
    build_returncode = run_logged_command(build_command, project_root, environment, emit_line, "build")
    if build_returncode != 0:
        return {
            "success": False,
            "status": "Build failed.",
            "target": normalized_target,
            "rebuildNeeded": rebuild_needed_before_build,
            "changedFileCount": changed_file_count_before_build,
            "changedFilesPreview": changed_files_preview_before_build,
            "rebuildTriggered": rebuild_needed_before_build,
        }

    set_target_last_success(server, normalized_target, time.time())

    executable = find_executable(build_dir)
    response = {
        "success": True,
        "status": "Build succeeded.",
        "buildDir": build_dir_relative.as_posix(),
        "target": normalized_target,
        "rebuildNeeded": False,
        "changedFileCount": 0,
        "changedFilesPreview": [],
        "rebuildTriggered": rebuild_needed_before_build,
    }
    if executable is not None:
        try:
            response["executable"] = executable.relative_to(project_root).as_posix()
        except ValueError:
            response["executable"] = executable.as_posix()

    return response


class BuildBridgeHandler(socketserver.StreamRequestHandler):
    def handle(self) -> None:
        write_lock = threading.Lock()
        while True:
            raw_request = self.rfile.readline().decode("utf-8", errors="replace")
            if not raw_request:
                return

            raw_request = raw_request.strip()
            if not raw_request:
                continue

            self.server.logger.comm_rx(raw_request)

            stream_closed = threading.Event()

            def emit_line(line: str) -> bool:
                if stream_closed.is_set():
                    return False

                if line.startswith("@@MYRIAD-BRIDGE-LOG@@"):
                    self.server.logger.build_output(line)
                else:
                    self.server.logger.comm_tx(line)

                try:
                    write_bridge_line(self.wfile, write_lock, line)
                    return True
                except (BrokenPipeError, ConnectionResetError, OSError):
                    stream_closed.set()
                    return False

            try:
                request = json.loads(raw_request)
            except json.JSONDecodeError as exc:
                payload = {"success": False, "status": f"Invalid JSON request: {exc}"}
                emit_line("@@MYRIAD-BRIDGE-RESPONSE@@ " + json.dumps(payload, separators=(",", ":")))
                if stream_closed.is_set():
                    return
                continue

            try:
                response = run_build(self.server, self.server.project_root, self.server.presets, request, emit_line)
            except Exception as exc:  # pragma: no cover - defensive server boundary
                response = {"success": False, "status": f"Build bridge error: {exc}"}

            emit_line("@@MYRIAD-BRIDGE-RESPONSE@@ " + json.dumps(response, separators=(",", ":")))
            if stream_closed.is_set():
                return


class BuildBridgeServer(socketserver.ThreadingTCPServer):
    allow_reuse_address = True

    def __init__(self, server_address, handler_class, project_root: Path, presets: dict[str, dict]):
        super().__init__(server_address, handler_class)
        self.project_root = project_root
        self.presets = presets
        self.build_state: dict[str, dict] = {}
        self.state_lock = threading.Lock()
        self.logger = BridgeTrafficLogger(project_root)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Myriad build bridge server")
    parser.add_argument("--host", default="0.0.0.0", help="Host interface to bind")
    parser.add_argument("--port", type=int, default=55333, help="TCP port to listen on")
    parser.add_argument("--project-root", default=None, help="Override the repository root")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.project_root is None:
        project_root = Path(__file__).resolve().parent.parent
    else:
        project_root = Path(args.project_root).resolve()

    presets = load_preset_map(project_root)
    if not presets:
        print("No compiler presets were found in CMakeKits.json.", file=sys.stderr)
        return 1

    with BuildBridgeServer((args.host, args.port), BuildBridgeHandler, project_root, presets) as server:
        print(f"[build-bridge] listening on {args.host}:{args.port} for {project_root}", flush=True)
        print(f"[build-bridge] logging traffic to {server.logger.path}", flush=True)
        server.logger.write("INFO", f"Bridge listening on {args.host}:{args.port} for {project_root}")
        try:
            server.serve_forever()
        except KeyboardInterrupt:
            print("[build-bridge] shutting down", flush=True)
            server.logger.write("INFO", "Bridge shutting down")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
