# Myriad

A game engine in C++

# Getting started

Requires docker installed withthe docker-buildx extension
`docker build -t myriad_dev docker/`

## Building

If on MINGW, you can alter whether libgcc is static or shared:
-DMYRIAD_MINGW_LIBGCC_SHARED=OFF in your CMake configure args, or
flip it in CMakeLists.txt:28.

It's a lot more convenient to build statically and not have to copy the dlls/sos around.

## External editor builds

If you launch the editor outside the devcontainer, the preferred path is to run the build bridge inside the container and point the editor at it over TCP. This works from Windows, WSL, and native Linux editor builds.

Start the bridge inside the devcontainer or Docker container:

`python3 docker/build_bridge_server.py --host 0.0.0.0 --port 55333`

Then set the editor to socket mode with `MYRIAD_BUILD_MODE=socket`, or add a `.myriad-editor.json` file at the repo root like this:

```json
{
  "buildMode": "socket",
  "buildSocketHosts": "192.168.1.50,127.0.0.1",
  "buildSocketHost": "127.0.0.1",
  "buildSocketPort": 55333,
  "buildBridgeProbeIntervalSeconds": 30
}
```

If `buildSocketHosts` is provided, the editor will try those hosts first, in order. Otherwise, if `buildSocketHost` is omitted or does not answer, the editor will try a small set of local bridge hosts automatically, including `127.0.0.1`, `localhost`, the WSL resolver nameserver IP, and common Docker host aliases, then cache the first one that responds.

The editor expands these placeholders when socket mode is not enabled and it falls back to a local build command:

- `{projectRoot}`: repository root path
- `{buildDir}`: selected build directory
- `{target}`: build target, currently `TestECS`
- `{toolchainArg}`: preset-specific `-DCMAKE_TOOLCHAIN_FILE=...` argument when available
- `{headerDirs}`: configured header directories, separated by semicolons or newlines
- `{libraryDirs}`: configured library directories, separated by semicolons or newlines
- `{includeArgs}`: configured header directories expanded as quoted `-I...` arguments
- `{libraryArgs}`: configured library directories expanded as quoted `-L...` arguments

The editor now supports a TCP build bridge instead of relying on local cmake when you enable socket mode. The client path lives in Editor/src/main.cpp, and the container-side listener is docker/build_bridge_server.py. The bridge also applies the preset environment variables from CMakeKits.json before it runs cmake, and I forwarded the bridge port in .devcontainer/devcontainer.json.

To use it from an external editor, run the bridge inside the container with python3 build_bridge_server.py --host 0.0.0.0 --port 55333, then set MYRIAD_BUILD_MODE=socket or add the socket settings shown in README.md.

For WSL or native Linux, keep the bridge host pointed at the local forwarded port unless your Docker setup exposes it differently.

Example launch command from inside the container shell:

`python3 /workspaces/Myriad/docker/build_bridge_server.py --host 0.0.0.0 --port 55333`

## Installed editor resources and config

When running an installed editor build, MyriadEditor looks for resource and config files in this order.

Project directory selection:

The project directory can be set in the editor under `Editor Preferences` -> `Project`. Header and library directories can be configured in the same panel. Those values are persisted in `.myriad-editor.json`.

Startup project root detection:

1. `MYRIAD_PROJECT_ROOT` (if set to a path inside a Myriad repo)
2. Current working directory or one of its parents, only when that path contains a Myriad repo
3. Persisted build/executable paths from editor settings

`MYRIAD_PROJECT_ROOT` is optional. It is useful only as a launch-time override; the editor does not require it once the project directory is configured in preferences.

Themes and layouts (`themes.json`, `layouts.json`):

1. `MYRIAD_EDITOR_RESOURCE_DIR` (if set)
2. Installed data directory resolved from the executable location (for example `<prefix>/share/myriad-editor`)
3. Source-tree fallback paths when running from the repository

Editor config (`.myriad-editor.json`) read order for source/build-tree runs:

1. `MYRIAD_EDITOR_CONFIG_PATH` (if set and file exists)
2. Project root `.myriad-editor.json` (source-tree runs)
3. Editor resource directory `.myriad-editor.json`

Editor config read order for installed editor runs:

1. `MYRIAD_EDITOR_CONFIG_PATH` (if set and file exists)
2. Editor resource directory `.myriad-editor.json`
3. Project root `.myriad-editor.json` as a fallback

Editor config write path:

1. `MYRIAD_EDITOR_CONFIG_PATH` (if set)
2. Project root `.myriad-editor.json` when running from source tree
3. Editor resource directory `.myriad-editor.json` for installed runs

Override examples:

Linux/macOS (bash):

`MYRIAD_PROJECT_ROOT=/workspaces/Myriad MYRIAD_EDITOR_RESOURCE_DIR=/opt/myriad/share/myriad-editor MYRIAD_EDITOR_CONFIG_PATH=/opt/myriad/share/myriad-editor/custom-editor.json /opt/myriad/bin/MyriadEditor`

Windows (PowerShell):

`$env:MYRIAD_PROJECT_ROOT='C:\src\Myriad'; $env:MYRIAD_EDITOR_RESOURCE_DIR='C:\Myriad\share\myriad-editor'; $env:MYRIAD_EDITOR_CONFIG_PATH='C:\Myriad\share\myriad-editor\custom-editor.json'; C:\Myriad\bin\MyriadEditor.exe`

## API documentation (Doxygen)

The repository includes a root Doxygen config at `Doxyfile`. The helper script generates separate Engine and Editor documentation under each project's `docs` directory.

Install Doxygen (Ubuntu/Debian):

`sudo apt-get update && sudo apt-get install -y doxygen`

Generate docs from the repository root:

`./scripts/generate-docs.sh`

The dev container serves the repository root on port `8000`, so generated docs are available at:

- `http://localhost:8000/Engine/docs/doxygen/html/index.html`
- `http://localhost:8000/Editor/docs/doxygen/html/index.html`

Generate and open the HTML docs in a browser:

`./scripts/generate-docs.sh --open`

Generated HTML entry point:

- `Engine/docs/doxygen/html/index.html`
- `Editor/docs/doxygen/html/index.html`
