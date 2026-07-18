#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage: scripts/generate-docs.sh [--open]

Generates Engine and Editor Doxygen docs using the repository Doxyfile.

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
engine_index_path="$repo_root/Engine/docs/doxygen/html/index.html"
editor_index_path="$repo_root/Editor/docs/doxygen/html/index.html"

if [[ ! -f "$config_path" ]]; then
  echo "Doxyfile not found at $config_path" >&2
  exit 1
fi

if ! command -v doxygen >/dev/null 2>&1; then
  echo "doxygen is not installed. Install it with: sudo apt-get update && sudo apt-get install -y doxygen" >&2
  exit 1
fi

cd "$repo_root"

generate_project_docs() {
  local project_name="$1"
  local project_brief="$2"
  local input_path="$3"
  local output_directory="$4"

  echo "Generating $project_name documentation..."
  mkdir -p "$repo_root/$output_directory"
  MYRIAD_DOXYGEN_PROJECT_NAME="$project_name" \
    MYRIAD_DOXYGEN_PROJECT_BRIEF="$project_brief" \
    MYRIAD_DOXYGEN_INPUT="$input_path" \
    MYRIAD_DOXYGEN_OUTPUT_DIRECTORY="$output_directory" \
    doxygen "$config_path"
}

generate_project_docs \
  "Myriad Engine" \
  "API documentation for the Myriad engine module" \
  "Engine/include Engine/src" \
  "Engine/docs/doxygen"

generate_project_docs \
  "Myriad Editor" \
  "API documentation for the Myriad editor module" \
  "Editor/src" \
  "Editor/docs/doxygen"

echo "Doxygen generation complete."
echo "Engine output: $engine_index_path"
echo "Editor output: $editor_index_path"

if [[ "$open_after_generate" == true ]]; then
  if [[ -f "$engine_index_path" && -f "$editor_index_path" ]]; then
    if [[ -n "${BROWSER:-}" ]]; then
      "$BROWSER" "file://$engine_index_path"
      "$BROWSER" "file://$editor_index_path"
    elif command -v xdg-open >/dev/null 2>&1; then
      xdg-open "$engine_index_path" >/dev/null 2>&1 || true
      xdg-open "$editor_index_path" >/dev/null 2>&1 || true
    else
      echo "No browser launcher found. Open manually:" >&2
      echo "  $engine_index_path" >&2
      echo "  $editor_index_path" >&2
    fi
  else
    echo "Generated docs were not found at the expected paths." >&2
    exit 1
  fi
fi
