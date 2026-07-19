#!/usr/bin/env python3
import argparse
import json
import os
import shutil
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
BUILD_TYPES = ["Debug", "Release", "RelWithDebInfo", "MinSizeRel"]


def normalize_display_root(path_text: str) -> str:
    normalized = str(path_text).strip().replace("\\", "/")
    while len(normalized) > 1 and normalized.endswith("/"):
        normalized = normalized[:-1]
    return normalized


def join_display_path(root_text: str, relative_path: Path) -> str:
    root = normalize_display_root(root_text)
    relative_text = relative_path.as_posix().strip("/")
    if not relative_text:
        return root
    if not root or root == "/":
        return "/" + relative_text
    return root + "/" + relative_text


def target_to_display(target_root: Path, source_root: str, target_path: Path) -> str:
    try:
        relative_path = target_path.resolve().relative_to(target_root)
    except ValueError:
        return target_path.as_posix()
    return join_display_path(source_root, relative_path)


def display_to_target(target_root: Path, source_root: str, path_text: str) -> Path:
    normalized_path = normalize_display_root(path_text)
    normalized_source = normalize_display_root(source_root)
    if normalized_source and (normalized_path == normalized_source or normalized_path.startswith(normalized_source + "/")):
        relative_text = normalized_path[len(normalized_source):].strip("/")
        return (target_root / relative_text).resolve()
    return Path(path_text)


def translate_path_list_to_target(server: "BuildBridgeServer", path_list_text: str) -> str:
    translated: list[str] = []
    for raw_part in str(path_list_text).replace("\r", "\n").replace(";", "\n").split("\n"):
        part = raw_part.strip()
        if not part:
            continue
        target_path = display_to_target(server.projects_root, server.projects_source_root, part)
        target_path = display_to_target(server.dist_root, server.dist_source_root, target_path.as_posix())
        translated.append(target_path.as_posix())
    return ";".join(translated)


def make_directory_defaults(server: "BuildBridgeServer", compiler_toolkit: str, build_type: str) -> dict:
    safe_toolkit = compiler_toolkit.strip() or "Default"
    safe_build_type = build_type.strip() or "Debug"
    header_target = server.dist_root / "myriad" / "include"
    library_target = server.dist_root / "myriad" / safe_toolkit / safe_build_type / "lib"
    return {
        "defaultHeaderSearchDirs": target_to_display(server.dist_root, server.dist_source_root, header_target),
        "defaultLibrarySearchDirs": target_to_display(server.dist_root, server.dist_source_root, library_target),
        "defaultHeaderSearchTargetDirs": header_target.as_posix(),
        "defaultLibrarySearchTargetDirs": library_target.as_posix(),
    }


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


def resolve_build_dir_path(server: "BuildBridgeServer", project_root: Path, project_path: str, path_text: str, preset_name: str, build_type: str) -> tuple[Path, Path]:
    trimmed_text = str(path_text).strip()
    if not trimmed_text:
        build_dir_relative = Path("build") / preset_name / build_type
        return build_dir_relative, (project_root / build_dir_relative).resolve()

    translated_path = display_to_target(server.projects_root, server.projects_source_root, trimmed_text)
    if translated_path.is_absolute():
        try:
            build_dir_relative = translated_path.resolve().relative_to(project_root)
        except ValueError as exc:
            raise ValueError("buildDir must stay inside the project root") from exc
        return build_dir_relative, translated_path.resolve()

    build_dir_relative = trim_relative_path(trimmed_text)
    return build_dir_relative, (project_root / build_dir_relative).resolve()


def trim_project_path(path_text: str) -> Path:
    normalized_text = path_text.strip().strip("/")
    if not normalized_text or normalized_text == ".":
        return Path("")

    path = Path(normalized_text)
    if path.is_absolute():
        raise ValueError("projectPath must be relative to the projects mount")

    if any(part == ".." for part in path.parts):
        raise ValueError("projectPath must stay inside the projects mount")

    return path


def resolve_mounted_project(projects_root: Path, path_text: str) -> Path:
    relative_path = trim_project_path(path_text)
    project_root = (projects_root / relative_path).resolve()

    try:
        project_root.relative_to(projects_root)
    except ValueError as exc:
        raise ValueError("projectPath must stay inside the projects mount") from exc

    if not project_root.exists() or not project_root.is_dir():
        raise ValueError(f"Project directory does not exist: {relative_path.as_posix()}")

    return project_root


def resolve_new_project_path(projects_root: Path, parent_text: str, name_text: str) -> tuple[Path, Path]:
    parent_path = trim_project_path(parent_text)
    project_name = name_text.strip().strip("/")
    if not project_name:
        raise ValueError("Project name is required")

    name_path = trim_project_path(project_name)
    if len(name_path.parts) != 1:
        raise ValueError("Project name must be a single directory name")

    project_path = parent_path / name_path
    project_root = (projects_root / project_path).resolve()
    try:
        project_root.relative_to(projects_root)
    except ValueError as exc:
        raise ValueError("projectPath must stay inside the projects mount") from exc

    return project_path, project_root


def resolve_new_directory_path(projects_root: Path, parent_text: str, name_text: str) -> tuple[Path, Path]:
    parent_path = trim_project_path(parent_text)
    directory_name = name_text.strip().strip("/")
    if not directory_name:
        raise ValueError("Directory name is required")

    name_path = trim_project_path(directory_name)
    if len(name_path.parts) != 1:
        raise ValueError("Directory name must be a single directory name")

    directory_path = parent_path / name_path
    directory_root = (projects_root / directory_path).resolve()
    try:
        directory_root.relative_to(projects_root)
    except ValueError as exc:
        raise ValueError("directory path must stay inside the projects mount") from exc

    return directory_path, directory_root


def resolve_export_directory(server: "BuildBridgeServer", path_text: str) -> tuple[Path, Path]:
    trimmed_text = str(path_text).strip()
    if not trimmed_text:
        raise ValueError("exportDirectory is empty")

    translated_path = display_to_target(server.projects_root, server.projects_source_root, trimmed_text)
    if translated_path.is_absolute():
        try:
            relative_path = translated_path.resolve().relative_to(server.projects_root)
        except ValueError as exc:
            raise ValueError("exportDirectory must stay inside the projects mount") from exc
        return relative_path, translated_path.resolve()

    relative_path = trim_project_path(trimmed_text)
    target_path = (server.projects_root / relative_path).resolve()
    try:
        target_path.relative_to(server.projects_root)
    except ValueError as exc:
        raise ValueError("exportDirectory must stay inside the projects mount") from exc
    return relative_path, target_path


def resolve_source_directory(server: "BuildBridgeServer", project_root: Path, path_text: str) -> Path:
    trimmed_text = str(path_text).strip()
    if not trimmed_text:
        default_src = project_root / "src"
        if not (project_root / "CMakeLists.txt").exists() and default_src.exists() and default_src.is_dir():
            return default_src
        return project_root

    translated_path = display_to_target(server.projects_root, server.projects_source_root, trimmed_text)
    source_directory = translated_path if translated_path.is_absolute() else (project_root / translated_path)
    source_directory = source_directory.resolve()
    try:
        source_directory.relative_to(project_root)
    except ValueError as exc:
        raise ValueError("sourceDirectory must stay inside the project root") from exc

    if not source_directory.exists() or not source_directory.is_dir():
        raise ValueError(f"sourceDirectory does not exist: {trimmed_text}")

    return source_directory


def cmake_quote_path(path: Path) -> str:
    return path.as_posix().replace('"', '\\"')


def write_generated_game_cmake_source(server: "BuildBridgeServer", source_directory: Path, build_dir: Path, project_name: str) -> Path:
    generated_source_dir = build_dir / ".myriad_bridge_cmake_source"
    generated_source_dir.mkdir(parents=True, exist_ok=True)
    shared_link = generated_source_dir / "shared"
    if not shared_link.exists():
        shared_link.symlink_to(server.project_root / "shared", target_is_directory=True)
    source_path = cmake_quote_path(source_directory)
    engine_path = cmake_quote_path(server.project_root / "Engine")
    generated_engine_build_path = cmake_quote_path(build_dir / ".myriad_bridge_engine")
    cmake_text = f'''cmake_minimum_required(VERSION 3.18.4)
project("{project_name}" VERSION 0.0.1 DESCRIPTION "Myriad generated game build")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(COMPILE_FLAGS -std=c++17 -Wall -Werror -fno-rtti -fno-exceptions)

option(MYRIAD_MINGW_LIBGCC_SHARED "Use shared libgcc on MinGW targets (OFF uses static libgcc)." OFF)
function(myriad_apply_mingw_libgcc target_name)
    if(MINGW)
        if(MYRIAD_MINGW_LIBGCC_SHARED)
            target_link_options(${{target_name}} PRIVATE -shared-libgcc -shared-libstdc++)
        else()
            target_link_options(${{target_name}} PRIVATE -static-libgcc -static-libstdc++)
        endif()
    endif()
endfunction()

set(MYRIAD_GAME_SOURCE_DIR "{source_path}" CACHE PATH "Game source directory")
add_subdirectory("{engine_path}" "{generated_engine_build_path}")

file(GLOB_RECURSE GAME_SOURCE_FILES CONFIGURE_DEPENDS
    "${{MYRIAD_GAME_SOURCE_DIR}}/*.c"
    "${{MYRIAD_GAME_SOURCE_DIR}}/*.cc"
    "${{MYRIAD_GAME_SOURCE_DIR}}/*.cpp"
    "${{MYRIAD_GAME_SOURCE_DIR}}/*.cxx")
set(GAME_MAIN_SOURCE "${{MYRIAD_GAME_SOURCE_DIR}}/main.cpp")
if(NOT EXISTS "${{GAME_MAIN_SOURCE}}")
    message(FATAL_ERROR "Game source directory is missing main.cpp: ${{MYRIAD_GAME_SOURCE_DIR}}")
endif()
list(FILTER GAME_SOURCE_FILES EXCLUDE REGEX ".*/main\\.cpp$")
if(NOT GAME_SOURCE_FILES)
    message(FATAL_ERROR "Game source directory has no library source files: ${{MYRIAD_GAME_SOURCE_DIR}}")
endif()

set(LIBRARY_NAME lib${{PROJECT_NAME}})
add_library(${{LIBRARY_NAME}} SHARED ${{GAME_SOURCE_FILES}})
myriad_apply_mingw_libgcc(${{LIBRARY_NAME}})
target_compile_options(${{LIBRARY_NAME}} PRIVATE ${{COMPILE_FLAGS}})
set_target_properties(${{LIBRARY_NAME}} PROPERTIES OUTPUT_NAME ${{PROJECT_NAME}})
target_include_directories(${{LIBRARY_NAME}} PUBLIC "${{MYRIAD_GAME_SOURCE_DIR}}" "{cmake_quote_path(server.project_root / 'Engine' / 'include')}")
target_link_libraries(${{LIBRARY_NAME}} MyriadEngine)

add_executable(${{PROJECT_NAME}} "${{GAME_MAIN_SOURCE}}")
myriad_apply_mingw_libgcc(${{PROJECT_NAME}})
target_compile_options(${{PROJECT_NAME}} PRIVATE ${{COMPILE_FLAGS}})
target_include_directories(${{PROJECT_NAME}} PUBLIC "${{MYRIAD_GAME_SOURCE_DIR}}" "{cmake_quote_path(server.project_root / 'Engine' / 'include')}")
target_link_libraries(${{PROJECT_NAME}} MyriadEngine ${{LIBRARY_NAME}})

if(WIN32)
    add_custom_command(TARGET ${{PROJECT_NAME}} POST_BUILD
        COMMAND ${{CMAKE_COMMAND}} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:${{PROJECT_NAME}}>
        $<TARGET_FILE_DIR:${{PROJECT_NAME}}>
        COMMAND_EXPAND_LISTS)
elseif(UNIX)
    add_custom_command(TARGET ${{PROJECT_NAME}} POST_BUILD
        COMMAND ${{CMAKE_COMMAND}} -E copy_if_different
        $<TARGET_FILE:MyriadEngine>
        $<TARGET_FILE_DIR:${{PROJECT_NAME}}>
        COMMAND_EXPAND_LISTS)
endif()
'''
    (generated_source_dir / "CMakeLists.txt").write_text(cmake_text, encoding="utf-8")
    return generated_source_dir


def prepare_cmake_source_directory(server: "BuildBridgeServer", source_directory: Path, build_dir: Path, project_name: str) -> Path:
    if (source_directory / "CMakeLists.txt").exists():
        return source_directory
    return write_generated_game_cmake_source(server, source_directory, build_dir, project_name)


def is_runtime_binary(path: Path) -> bool:
    suffix = path.suffix.lower()
    if suffix in {".exe", ".dll", ".so", ".dylib"}:
        return True
    if ".so." in path.name.lower():
        return True
    return suffix == "" and path.is_file() and os.access(path, os.X_OK)


def copy_runtime_outputs(output_directory: Path, export_directory: Path) -> list[Path]:
    copied: list[Path] = []
    if not output_directory.exists() or not output_directory.is_dir():
        return copied

    for candidate in sorted(output_directory.iterdir(), key=lambda item: item.name.lower()):
        if not candidate.is_file() or not is_runtime_binary(candidate):
            continue

        destination = export_directory / candidate.name
        shutil.copy2(candidate, destination)
        copied.append(destination)

    return copied


def copy_resource_directory(source: Path, destination: Path) -> list[Path]:
    if not source.exists() or not source.is_dir():
        return []

    if destination.exists():
        shutil.rmtree(destination)
    shutil.copytree(source, destination)
    return [destination]


def copy_resources_to_export(project_root: Path, source_directory: Path, export_directory: Path) -> list[Path]:
    copied: list[Path] = []
    copied.extend(copy_resource_directory(project_root / "shared" / "res", export_directory / "shared" / "res"))
    for directory_name in ("res", "resources", "assets"):
        copied.extend(copy_resource_directory(source_directory / directory_name, export_directory / directory_name))
    return copied


def export_build_outputs(server: "BuildBridgeServer", project_root: Path, source_directory: Path, executable: Path, export_directory_text: str) -> dict | None:
    if not export_directory_text.strip():
        return None

    export_relative_path, export_directory = resolve_export_directory(server, export_directory_text)
    export_directory.mkdir(parents=True, exist_ok=True)
    copied_runtime_files = copy_runtime_outputs(executable.parent, export_directory)
    if not any(path.name == executable.name for path in copied_runtime_files):
        exported_executable = export_directory / executable.name
        shutil.copy2(executable, exported_executable)
        copied_runtime_files.append(exported_executable)
    copied_resource_dirs = copy_resources_to_export(project_root, source_directory, export_directory)
    return {
        "exportDirectory": export_directory,
        "exportedExecutable": export_directory / executable.name,
        "exportedRuntimeFiles": copied_runtime_files,
        "exportedResourceDirectories": copied_resource_dirs,
    }


def create_cmake_configure_command(cmake_source_directory: Path, build_dir: Path, build_type: str, project_name: str, source_directory: Path, toolchain_path: Path | None) -> list[str]:
    command = [
        "cmake",
        "-S",
        str(cmake_source_directory),
        "-B",
        str(build_dir),
        f"-DCMAKE_BUILD_TYPE={build_type}",
        f"-DMYRIAD_GAME_PROJECT_NAME={project_name}",
        f"-DMYRIAD_GAME_SOURCE_DIR={source_directory}",
    ]
    if toolchain_path is not None:
        command.append(f"-DCMAKE_TOOLCHAIN_FILE={toolchain_path}")
    return command


def create_cmake_build_command(build_dir: Path, project_name: str) -> list[str]:
    return ["cmake", "--build", str(build_dir), "--target", project_name]


def write_initial_project_config(project_root: Path, project_path: Path, bridge_port: int, compiler_toolkit: str) -> Path:
    config_dir = project_root / ".myriad_editor"
    config_dir.mkdir(parents=True, exist_ok=True)
    config_path = config_dir / "editor.json"
    if not config_path.exists():
        config = {
            "projectMountPath": project_path.as_posix(),
            "buildMode": "socket",
            "buildSocketHost": "127.0.0.1",
            "buildSocketPort": bridge_port,
            "compilerToolkit": compiler_toolkit,
            "buildType": "Debug",
        }
        config_path.write_text(json.dumps(config, indent=2) + "\n", encoding="utf-8")

    return config_path


def load_project_presets(project_root: Path, fallback_presets: dict[str, dict]) -> dict[str, dict]:
    kits_path = project_root / "CMakeKits.json"
    if kits_path.exists():
        return load_preset_map(project_root)

    return fallback_presets


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


def find_executable(build_dir: Path, project_name: str) -> Path | None:
    normalized_project_name = project_name.strip() or "TestECS"
    target_names = {normalized_project_name.lower(), f"{normalized_project_name}.exe".lower()}
    search_roots = [
        build_dir / "Examples" / normalized_project_name,
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


def iter_watch_roots(project_root: Path, normalized_target: str, source_directory: Path | None = None) -> list[Path]:
    roots = [
        project_root / "Engine",
        project_root / "shared",
    ]

    if source_directory is not None:
        roots.append(source_directory)
        return roots

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


def scan_rebuild_state(project_root: Path, normalized_target: str, last_success_build_time: float, source_directory: Path | None = None) -> tuple[bool, int, list[str]]:
    changed_file_count = 0
    changed_files_preview: list[str] = []

    for root in iter_watch_roots(project_root, normalized_target, source_directory):
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


def get_target_state(server: "BuildBridgeServer", project_path: str, normalized_target: str) -> dict:
    state_key = f"{project_path}:{normalized_target}"
    with server.state_lock:
        state = server.build_state.get(state_key)
        if state is None:
            state = {"lastSuccessBuildTime": 0.0}
            server.build_state[state_key] = state
        return dict(state)


def set_target_last_success(server: "BuildBridgeServer", project_path: str, normalized_target: str, success_time: float) -> None:
    state_key = f"{project_path}:{normalized_target}"
    with server.state_lock:
        state = server.build_state.get(state_key)
        if state is None:
            state = {"lastSuccessBuildTime": 0.0}
            server.build_state[state_key] = state
        state["lastSuccessBuildTime"] = success_time


def make_status_response(server: "BuildBridgeServer", project_root: Path, presets: dict[str, dict], normalized_target: str, last_success_build_time: float, project_path: str = "", source_directory: Path | None = None) -> dict:
    rebuild_needed, changed_file_count, changed_files_preview = scan_rebuild_state(project_root, normalized_target, last_success_build_time, source_directory)
    return {
        "success": True,
        "status": "Bridge reachable.",
        "bridge": "online",
        "projectRoot": project_root.as_posix(),
        "sourceDirectory": target_to_display(server.projects_root, server.projects_source_root, source_directory) if source_directory is not None and project_path else (source_directory.as_posix() if source_directory is not None else project_root.as_posix()),
        "projectPath": project_path,
        "presetCount": len(presets),
        "target": normalized_target,
        "rebuildNeeded": rebuild_needed,
        "changedFileCount": changed_file_count,
        "changedFilesPreview": changed_files_preview,
    }


def list_projects(server: "BuildBridgeServer", request: dict) -> dict:
    relative_path = trim_project_path(str(request.get("path", "")))
    directory = (server.projects_root / relative_path).resolve()

    try:
        directory.relative_to(server.projects_root)
    except ValueError as exc:
        raise ValueError("path must stay inside the projects mount") from exc

    if not directory.exists() or not directory.is_dir():
        return {
            "success": False,
            "status": f"Project browser path does not exist: {relative_path.as_posix()}",
            "projectMount": server.projects_source_root,
            "projectMountSource": server.projects_source_root,
            "projectMountTarget": server.projects_root.as_posix(),
            "path": "" if relative_path == Path("") else relative_path.as_posix(),
            "directories": [],
        }

    directories: list[str] = []
    for child in sorted(directory.iterdir(), key=lambda item: item.name.lower()):
        if child.is_dir() and not child.name.startswith("."):
            directories.append(child.name)

    parent = ""
    if relative_path != Path(""):
        parent_path = relative_path.parent
        parent = "" if parent_path == Path(".") else parent_path.as_posix()

    return {
        "success": True,
        "status": "Projects listed.",
        "projectMount": server.projects_source_root,
        "projectMountSource": server.projects_source_root,
        "projectMountTarget": server.projects_root.as_posix(),
        "path": "" if relative_path == Path("") else relative_path.as_posix(),
        "parent": parent,
        "directories": directories,
    }


def list_directories(server: "BuildBridgeServer", request: dict) -> dict:
    relative_path = trim_project_path(str(request.get("path", "")))
    directory = (server.projects_root / relative_path).resolve()

    try:
        directory.relative_to(server.projects_root)
    except ValueError as exc:
        raise ValueError("path must stay inside the projects mount") from exc

    if not directory.exists() or not directory.is_dir():
        return {
            "success": False,
            "status": f"Directory browser path does not exist: {relative_path.as_posix()}",
            "projectMount": server.projects_source_root,
            "projectMountSource": server.projects_source_root,
            "projectMountTarget": server.projects_root.as_posix(),
            "path": "" if relative_path == Path("") else relative_path.as_posix(),
            "directories": [],
        }

    directories: list[str] = []
    for child in sorted(directory.iterdir(), key=lambda item: item.name.lower()):
        if child.is_dir() and not child.name.startswith("."):
            directories.append(child.name)

    parent = ""
    if relative_path != Path(""):
        parent_path = relative_path.parent
        parent = "" if parent_path == Path(".") else parent_path.as_posix()

    return {
        "success": True,
        "status": "Directories listed.",
        "projectMount": server.projects_source_root,
        "projectMountSource": server.projects_source_root,
        "projectMountTarget": server.projects_root.as_posix(),
        "path": "" if relative_path == Path("") else relative_path.as_posix(),
        "parent": parent,
        "directories": directories,
    }


def open_project(server: "BuildBridgeServer", request: dict) -> dict:
    project_path = trim_project_path(str(request.get("projectPath", "")))
    project_root = resolve_mounted_project(server.projects_root, project_path.as_posix())
    settings_dir = project_root / ".myriad_editor"
    if not settings_dir.exists() or not settings_dir.is_dir():
        return {
            "success": False,
            "status": f"Error: Project is missing .myriad_editor directory: {project_path.as_posix()}",
            "projectMount": server.projects_source_root,
            "projectMountSource": server.projects_source_root,
            "projectMountTarget": server.projects_root.as_posix(),
            "projectPath": project_path.as_posix(),
            "projectRoot": project_root.as_posix(),
            "projectDisplayRoot": target_to_display(server.projects_root, server.projects_source_root, project_root),
        }

    presets = load_project_presets(project_root, server.presets)
    return {
        "success": True,
        "status": "Project opened.",
        "projectMount": server.projects_source_root,
        "projectMountSource": server.projects_source_root,
        "projectMountTarget": server.projects_root.as_posix(),
        "projectPath": project_path.as_posix(),
        "projectRoot": project_root.as_posix(),
        "projectDisplayRoot": target_to_display(server.projects_root, server.projects_source_root, project_root),
        "settingsPath": (project_root / ".myriad_editor" / "editor.json").as_posix(),
        "presetCount": len(presets),
    }


def list_build_options(server: "BuildBridgeServer", request: dict) -> dict:
    project_root, project_path, presets = resolve_request_project(server, request)
    requested_toolkit = str(request.get("compilerToolkit", "")).strip()
    selected_toolkit = requested_toolkit if requested_toolkit in presets else next(iter(presets.keys()), "Default")
    requested_build_type = str(request.get("buildType", "Debug")).strip() or "Debug"
    selected_build_type = requested_build_type if requested_build_type in BUILD_TYPES else BUILD_TYPES[0]
    response = {
        "success": True,
        "status": "Build options listed.",
        "projectRoot": project_root.as_posix(),
        "projectDisplayRoot": target_to_display(server.projects_root, server.projects_source_root, project_root) if project_path else project_root.as_posix(),
        "projectMount": server.projects_source_root,
        "projectMountSource": server.projects_source_root,
        "projectMountTarget": server.projects_root.as_posix(),
        "distMountSource": server.dist_source_root,
        "distMountTarget": server.dist_root.as_posix(),
        "projectPath": project_path,
        "compilerToolkits": list(presets.keys()),
        "buildTypes": BUILD_TYPES,
        "compilerToolkit": selected_toolkit,
        "buildType": selected_build_type,
    }
    response.update(make_directory_defaults(server, selected_toolkit, selected_build_type))
    return response


def create_project(server: "BuildBridgeServer", request: dict) -> dict:
    project_path, project_root = resolve_new_project_path(
        server.projects_root,
        str(request.get("parent", "")),
        str(request.get("name", "")),
    )
    if project_root.exists():
        raise ValueError(f"Project directory already exists: {project_path.as_posix()}")

    presets = load_project_presets(project_root, server.presets)
    default_toolkit = next(iter(presets.keys()), "")
    project_root.mkdir(parents=True, exist_ok=False)
    settings_path = write_initial_project_config(project_root, project_path, int(server.server_address[1]), default_toolkit)
    return {
        "success": True,
        "status": "Project created.",
        "projectMount": server.projects_source_root,
        "projectMountSource": server.projects_source_root,
        "projectMountTarget": server.projects_root.as_posix(),
        "projectPath": project_path.as_posix(),
        "projectRoot": project_root.as_posix(),
        "projectDisplayRoot": target_to_display(server.projects_root, server.projects_source_root, project_root),
        "settingsPath": settings_path.as_posix(),
        "presetCount": len(presets),
    }


def create_directory(server: "BuildBridgeServer", request: dict) -> dict:
    directory_path, directory_root = resolve_new_directory_path(
        server.projects_root,
        str(request.get("parent", "")),
        str(request.get("name", "")),
    )
    if directory_root.exists():
        raise ValueError(f"Directory already exists: {directory_path.as_posix()}")

    directory_root.mkdir(parents=True, exist_ok=False)
    return {
        "success": True,
        "status": "Directory created.",
        "projectMount": server.projects_source_root,
        "projectMountSource": server.projects_source_root,
        "projectMountTarget": server.projects_root.as_posix(),
        "path": directory_path.as_posix(),
        "directoryDisplayRoot": target_to_display(server.projects_root, server.projects_source_root, directory_root),
        "directoryTargetRoot": directory_root.as_posix(),
    }


def save_project_settings(server: "BuildBridgeServer", request: dict) -> dict:
    project_path = trim_project_path(str(request.get("projectPath", "")))
    if project_path == Path(""):
        raise ValueError("projectPath is required to save project settings")

    project_root = resolve_mounted_project(server.projects_root, project_path.as_posix())
    settings = request.get("settings")
    if not isinstance(settings, dict):
        raise ValueError("settings must be a JSON object")

    settings_dir = project_root / ".myriad_editor"
    settings_dir.mkdir(parents=True, exist_ok=True)
    settings_path = settings_dir / "editor.json"
    settings_path.write_text(json.dumps(settings, indent=2) + "\n", encoding="utf-8")
    return {
        "success": True,
        "status": "Project settings saved.",
        "projectMount": server.projects_source_root,
        "projectMountSource": server.projects_source_root,
        "projectMountTarget": server.projects_root.as_posix(),
        "projectPath": project_path.as_posix(),
        "projectRoot": project_root.as_posix(),
        "projectDisplayRoot": target_to_display(server.projects_root, server.projects_source_root, project_root),
        "settingsPath": settings_path.as_posix(),
    }


def resolve_request_project(server: "BuildBridgeServer", request: dict) -> tuple[Path, str, dict[str, dict]]:
    project_path = trim_project_path(str(request.get("projectPath", "")))
    if project_path == Path(""):
        return server.project_root, "", server.presets

    project_root = resolve_mounted_project(server.projects_root, project_path.as_posix())
    return project_root, project_path.as_posix(), load_project_presets(project_root, server.presets)


def run_build(server: "BuildBridgeServer", request: dict, emit_line) -> dict:
    action = str(request.get("action", "")).strip().lower()
    if action == "listprojects":
        return list_projects(server, request)

    if action == "openproject":
        return open_project(server, request)

    if action == "createproject":
        return create_project(server, request)

    if action == "listdirectories":
        return list_directories(server, request)

    if action == "createdirectory":
        return create_directory(server, request)

    if action == "saveprojectsettings":
        return save_project_settings(server, request)

    if action == "listbuildoptions":
        return list_build_options(server, request)

    project_root, project_path, presets = resolve_request_project(server, request)
    project_name = str(request.get("projectName", request.get("target", "TestECS"))).strip() or "TestECS"
    normalized_target = normalize_target_name(project_name)
    target_state = get_target_state(server, project_path, normalized_target)
    last_success_build_time = float(target_state.get("lastSuccessBuildTime", 0.0))

    status_source_directory = resolve_source_directory(server, project_root, str(request.get("sourceDirectory", "")))
    if action == "status":
        return make_status_response(server, project_root, presets, normalized_target, last_success_build_time, project_path, status_source_directory)

    preset_name = str(request.get("compilerToolkit", request.get("preset", ""))).strip()
    if not preset_name:
        return {"success": False, "status": "Missing compiler toolkit name."}

    preset = presets.get(preset_name)
    if preset is None:
        return {"success": False, "status": f"Unknown compiler toolkit: {preset_name}"}

    build_type = str(request.get("buildType", "Debug")).strip() or "Debug"
    if build_type not in BUILD_TYPES:
        return {"success": False, "status": f"Unknown build type: {build_type}"}

    environment = resolve_preset_environment(preset)

    source_directory = status_source_directory
    build_dir_relative, build_dir = resolve_build_dir_path(server, project_root, project_path, str(request.get("buildDir", "")), preset_name, build_type)

    rebuild_needed_before_build, changed_file_count_before_build, changed_files_preview_before_build = scan_rebuild_state(project_root, normalized_target, last_success_build_time, source_directory)
    build_dir.mkdir(parents=True, exist_ok=True)

    toolchain_path = resolve_toolchain_path(project_root, str(preset.get("toolchainFile", "")))
    cmake_source_directory = prepare_cmake_source_directory(server, source_directory, build_dir, project_name)
    configure_command = create_cmake_configure_command(cmake_source_directory, build_dir, build_type, project_name, source_directory, toolchain_path)

    configure_returncode = run_logged_command(configure_command, project_root, environment, emit_line, "configure")
    if configure_returncode != 0:
        return {"success": False, "status": "Configure failed."}

    build_command = create_cmake_build_command(build_dir, project_name)
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

    set_target_last_success(server, project_path, normalized_target, time.time())

    executable = find_executable(build_dir, project_name)
    response = {
        "success": True,
        "status": "Build succeeded.",
        "buildDir": build_dir_relative.as_posix(),
        "sourceDirectory": target_to_display(server.projects_root, server.projects_source_root, source_directory) if project_path else source_directory.as_posix(),
        "compilerToolkit": preset_name,
        "buildType": build_type,
        "projectName": project_name,
        "headerSearchDirs": translate_path_list_to_target(server, str(request.get("headerSearchDirs", ""))),
        "librarySearchDirs": translate_path_list_to_target(server, str(request.get("librarySearchDirs", ""))),
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

        exported = export_build_outputs(server, project_root, source_directory, executable, str(request.get("exportDirectory", "")))
        if exported is not None:
            response["exportDirectory"] = target_to_display(server.projects_root, server.projects_source_root, exported["exportDirectory"])
            response["exportedExecutable"] = target_to_display(server.projects_root, server.projects_source_root, exported["exportedExecutable"])
            response["exportedRuntimeFiles"] = [target_to_display(server.projects_root, server.projects_source_root, path) for path in exported["exportedRuntimeFiles"]]
            response["exportedResourceDirectories"] = [target_to_display(server.projects_root, server.projects_source_root, path) for path in exported["exportedResourceDirectories"]]

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
                response = run_build(self.server, request, emit_line)
            except Exception as exc:  # pragma: no cover - defensive server boundary
                response = {"success": False, "status": f"Build bridge error: {exc}"}

            emit_line("@@MYRIAD-BRIDGE-RESPONSE@@ " + json.dumps(response, separators=(",", ":")))
            if stream_closed.is_set():
                return


class BuildBridgeServer(socketserver.ThreadingTCPServer):
    allow_reuse_address = True

    def __init__(self, server_address, handler_class, project_root: Path, projects_root: Path, projects_source_root: str, dist_root: Path, dist_source_root: str, presets: dict[str, dict]):
        super().__init__(server_address, handler_class)
        self.project_root = project_root
        self.projects_root = projects_root
        self.projects_source_root = normalize_display_root(projects_source_root)
        self.dist_root = dist_root
        self.dist_source_root = normalize_display_root(dist_source_root)
        self.presets = presets
        self.build_state: dict[str, dict] = {}
        self.state_lock = threading.Lock()
        self.logger = BridgeTrafficLogger(project_root)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Myriad build bridge server")
    parser.add_argument("--host", default="0.0.0.0", help="Host interface to bind")
    parser.add_argument("--port", type=int, default=55333, help="TCP port to listen on")
    parser.add_argument("--project-root", default=None, help="Override the repository root")
    parser.add_argument("--projects-root", default=os.environ.get("MYRIAD_PROJECTS_ROOT", "/workspaces/projects"), help="Mounted projects directory")
    parser.add_argument("--projects-source-root", default=os.environ.get("MYRIAD_PROJECTS_SOURCE_ROOT", os.environ.get("MYRIAD_PROJECTS_ROOT", "/workspaces/projects")), help="Source-side display root for the mounted projects directory")
    parser.add_argument("--dist-root", default=os.environ.get("MYRIAD_DIST_ROOT", "/workspaces/dist"), help="Mounted Myriad distribution directory")
    parser.add_argument("--dist-source-root", default=os.environ.get("MYRIAD_DIST_SOURCE_ROOT", os.environ.get("MYRIAD_DIST_ROOT", "/workspaces/dist")), help="Source-side display root for the mounted distribution directory")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.project_root is None:
        project_root = Path(__file__).resolve().parent.parent
    else:
        project_root = Path(args.project_root).resolve()

    projects_root = Path(args.projects_root).resolve()
    dist_root = Path(args.dist_root).resolve()

    presets = load_preset_map(project_root)
    if not presets:
        print("No compiler presets were found in CMakeKits.json.", file=sys.stderr)
        return 1

    with BuildBridgeServer((args.host, args.port), BuildBridgeHandler, project_root, projects_root, args.projects_source_root, dist_root, args.dist_source_root, presets) as server:
        print(f"[build-bridge] listening on {args.host}:{args.port} for {project_root}", flush=True)
        print(f"[build-bridge] projects mount: {server.projects_source_root} -> {projects_root}", flush=True)
        print(f"[build-bridge] dist mount: {server.dist_source_root} -> {dist_root}", flush=True)
        print(f"[build-bridge] logging traffic to {server.logger.path}", flush=True)
        server.logger.write("INFO", f"Bridge listening on {args.host}:{args.port} for {project_root}")
        server.logger.write("INFO", f"Projects mount: {server.projects_source_root} -> {projects_root}")
        server.logger.write("INFO", f"Dist mount: {server.dist_source_root} -> {dist_root}")
        try:
            server.serve_forever()
        except KeyboardInterrupt:
            print("[build-bridge] shutting down", flush=True)
            server.logger.write("INFO", "Bridge shutting down")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
