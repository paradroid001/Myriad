#!/usr/bin/env bash
set -euo pipefail
if [ "$#" -lt 1 ]; then
  echo "Usage: $0 <path-to-windows-exe> [args...]" >&2
  exit 2
fi
exe="$1"
shift
if command -v cmd.exe >/dev/null 2>&1; then
  exec cmd.exe /c "$(wslpath -w "$exe")" "$@"
elif command -v wine >/dev/null 2>&1; then
  exec wine "$exe" "$@"
elif [ -x "$exe" ]; then
  exec "$exe" "$@"
else
  echo "No Windows executable runner found. Install wine or enable Windows path integration from WSL." >&2
  exit 1
fi
