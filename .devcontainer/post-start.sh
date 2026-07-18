#!/bin/bash
set -euo pipefail

repo_root="/workspaces/Myriad"
log_dir="$repo_root/runtime_logs"
docs_pattern='[p]ython3 -m http.server 8000 .*--directory /workspaces/Myriad'
bridge_pattern='[p]ython3 docker/build_bridge_server.py --host 0.0.0.0 --port 55333'
mkdir -p "$log_dir"
cd "$repo_root"
touch "$log_dir/docs-http-server.log" "$log_dir/build-bridge-server.log"

start_service() {
	local name="$1"
	local pattern="$2"
	local log_file="$3"
	shift 3

	if pgrep -f "$pattern" >/dev/null; then
		echo "$name is already running."
		return 0
	fi

	echo "Starting $name..."
	nohup "$@" >"$log_file" 2>&1 &
}

wait_for_build_bridge() {
	local attempt

	for attempt in {1..20}; do
		if timeout 1 bash -c '</dev/tcp/127.0.0.1/55333' >/dev/null 2>&1; then
			return 0
		fi
		sleep 0.25
	done

	return 1
}

write_build_bridge_status() {
	local pid_text
	local pid
	local listening="false"
	local traffic_log=""
	pid_text="$(pgrep -f "$bridge_pattern" || true)"
	pid="${pid_text%%$'\n'*}"
	traffic_log="$(find "$log_dir" -maxdepth 1 -type f -name 'build-bridge-[0-9]*.log' -printf '%T@ %p\n' 2>/dev/null | sort -nr | awk 'NR == 1 {print $2}')"

	if [[ -n "$traffic_log" ]]; then
		traffic_log="runtime_logs/${traffic_log##*/}"
	else
		traffic_log="runtime_logs/build-bridge-server.log"
	fi

	if timeout 1 bash -c '</dev/tcp/127.0.0.1/55333' >/dev/null 2>&1; then
		listening="true"
	fi

	cat >"$log_dir/build-bridge-status.json" <<EOF
{
  "name": "build bridge server",
  "running": $([[ -n "$pid" ]] && echo true || echo false),
  "listening": $listening,
  "pid": "$pid",
  "host": "127.0.0.1",
  "port": 55333,
  "checkedAt": "$(date -Iseconds)",
	"log": "runtime_logs/build-bridge-server.log",
	"trafficLog": "$traffic_log"
}
EOF
}

start_service \
	"documentation server" \
	"$docs_pattern" \
	"$log_dir/docs-http-server.log" \
	python3 -m http.server 8000 --bind 0.0.0.0 --directory "$repo_root"

start_service \
	"build bridge server" \
	"$bridge_pattern" \
	"$log_dir/build-bridge-server.log" \
	python3 docker/build_bridge_server.py --host 0.0.0.0 --port 55333

wait_for_build_bridge || true
write_build_bridge_status
