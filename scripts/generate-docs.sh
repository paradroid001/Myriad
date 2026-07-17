#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage: scripts/generate-docs.sh [--open]

Generates Doxygen docs using the repository Doxyfile.

Options:
  --open    Open the generated HTML index in a browser after generation.
  -h, --help Show this help message.
EOF
}

open_after_generate=false
for arg in "$@"; do
  case "$arg" in
    --open)
      open_after_generate=true
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown argument: $arg" >&2
      usage >&2
      exit 1
      ;;
  esac
done

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "$script_dir/.." && pwd)"
config_path="$repo_root/Doxyfile"
index_path="$repo_root/docs/doxygen/html/index.html"

if [[ ! -f "$config_path" ]]; then
  echo "Doxyfile not found at $config_path" >&2
  exit 1
fi

if ! command -v doxygen >/dev/null 2>&1; then
  echo "doxygen is not installed. Install it with: sudo apt-get update && sudo apt-get install -y doxygen" >&2
  exit 1
fi

cd "$repo_root"
doxygen "$config_path"

echo "Doxygen generation complete."
echo "Output: $index_path"

if [[ "$open_after_generate" == true ]]; then
  if [[ -f "$index_path" ]]; then
    if [[ -n "${BROWSER:-}" ]]; then
      "$BROWSER" "file://$index_path"
    elif command -v xdg-open >/dev/null 2>&1; then
      xdg-open "$index_path" >/dev/null 2>&1 || true
    else
      echo "No browser launcher found. Open manually: $index_path" >&2
    fi
  else
    echo "Generated index not found at $index_path" >&2
    exit 1
  fi
fi
