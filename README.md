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

Then set the editor to socket mode with `MYRIAD_BUILD_MODE=socket`, or add `.myriad_editor/editor.json` under the project root like this:

```json
{
  "build": {
    "buildMode": "socket",
    "buildSocketHosts": "192.168.1.50,127.0.0.1",
    "buildSocketHost": "127.0.0.1",
    "buildSocketPort": 55333,
    "buildBridgeProbeIntervalSeconds": 30
  }
}
```

If `buildSocketHosts` is provided, the editor will try those hosts first, in order. Otherwise, if `buildSocketHost` is omitted or does not answer, the editor will try a small set of local bridge hosts automatically, including `127.0.0.1`, `localhost`, the WSL resolver nameserver IP, and common Docker host aliases, then cache the first one that responds.

The editor expands these placeholders when socket mode is not enabled and it falls back to a local build command:

- `{projectRoot}`: repository root path
- `{buildDir}`: selected build directory
- `{target}`: configured target executable name
- `{toolchainArg}`: preset-specific `-DCMAKE_TOOLCHAIN_FILE=...` argument when available
- `{headerDirs}`: configured header directories, separated by semicolons or newlines
- `{libraryDirs}`: configured library directories, separated by semicolons or newlines
- `{includeArgs}`: configured header directories expanded as quoted `-I...` arguments
- `{libraryArgs}`: configured library directories expanded as quoted `-L...` arguments

Project metadata is stored separately from build output names. `project.projectName` is the editor-facing project label, while `build.targetExecutableName` controls generated bridge targets, local `{target}` expansion, and executable discovery. `build.sourceDirectory` and `build.buildDirectory` are persisted relative to the configured project root when they point inside it; legacy absolute values remain readable.

The editor now supports a TCP build bridge instead of relying on local cmake when you enable socket mode. The client path lives in Editor/src/main.cpp, and the container-side listener is docker/build_bridge_server.py. The bridge also applies the preset environment variables from CMakeKits.json before it runs cmake, and I forwarded the bridge port in .devcontainer/devcontainer.json.

To use it from an external editor, run the bridge inside the container with python3 build_bridge_server.py --host 0.0.0.0 --port 55333, then set MYRIAD_BUILD_MODE=socket or add the socket settings shown in README.md.

For WSL or native Linux, keep the bridge host pointed at the local forwarded port unless your Docker setup exposes it differently.

Example launch command from inside the container shell:

`python3 /workspaces/Myriad/docker/build_bridge_server.py --host 0.0.0.0 --port 55333`

## Installed editor resources and config

When running an installed editor build, MyriadEditor looks for resource and config files in this order.

Project directory selection:

The project directory can be set in the editor under `Project` -> `Settings`. Header and library directories can be configured in the same panel. Those values are persisted in grouped sections in `.myriad_editor/editor.json` under the current project.

Startup project root detection:

1. `MYRIAD_PROJECT_ROOT` (if set to a path inside a Myriad repo)
2. Current working directory or one of its parents, only when that path contains a Myriad repo
3. Persisted build/executable paths from editor settings

`MYRIAD_PROJECT_ROOT` is optional. It is useful only as a launch-time override; the editor does not require it once the project directory is configured in preferences.

Themes and layouts (`themes.json`, `layouts.json`):

1. `MYRIAD_EDITOR_RESOURCE_DIR` (if set)
2. Installed data directory resolved from the executable location (for example `<prefix>/myriad/share/myriad-editor`)
3. Source-tree fallback paths when running from the repository

Editor config read order for source/build-tree runs:

1. `MYRIAD_EDITOR_CONFIG_PATH` (if set and file exists)
2. Project root `.myriad_editor/editor.json`
3. Project root `.myriad-editor.json` legacy fallback
4. Editor resource directory `.myriad-editor.json`

Editor config read order for installed editor runs:

1. `MYRIAD_EDITOR_CONFIG_PATH` (if set and file exists)
2. Project root `.myriad_editor/editor.json`
3. Project root `.myriad-editor.json` legacy fallback
4. Editor resource directory `.myriad-editor.json`

Editor config write path:

1. `MYRIAD_EDITOR_CONFIG_PATH` (if set)
2. Project root `.myriad_editor/editor.json` when a project root is active
3. Editor resource directory `.myriad-editor.json`

Newly written config files use grouped `build`, `project`, `ui`, `last`, and `panels` sections. Existing flat files remain readable while projects migrate.

Override examples:

Linux/macOS (bash):

`MYRIAD_PROJECT_ROOT=/workspaces/Myriad MYRIAD_EDITOR_RESOURCE_DIR=/opt/myriad/share/myriad-editor MYRIAD_EDITOR_CONFIG_PATH=/opt/myriad/share/myriad-editor/custom-editor.json "/opt/myriad/G++ 11.4.0 x86_64-linux-gnu/Debug/bin/MyriadEditor"`

Windows (PowerShell):

`$env:MYRIAD_PROJECT_ROOT='C:\src\Myriad'; $env:MYRIAD_EDITOR_RESOURCE_DIR='C:\Myriad\share\myriad-editor'; $env:MYRIAD_EDITOR_CONFIG_PATH='C:\Myriad\share\myriad-editor\custom-editor.json'; & 'C:\Myriad\MinGW 10 x86_64 windows\Debug\bin\MyriadEditor.exe'`

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

# Issues

As of 20260720 we have an Editor which has been largely coded by AI. It communicates with the workspace/devcontainer via a python 'build bridge' exposed on a socket, so that it can trigger builds. This largely works, and - on linux - a linux (G++) build can be done and run, as well as previewed.
This does have a number of issues though.

1. It doesn't work properly on windows - especially building: this mainly seems to be a pathing issue for projects located in the wsl system.
2. It builds and runs on linux, and preview works, but I don't exactly understand how because the hosted preview code in NyrEntryPoint.cpp seems to be behind an ifdef.
3. I have no idea how the build bridge is architected. I also asked for the Engine to use cereal for serialisation but I don't think it is, or it's still heavily reliant on the json utils it made.
4. I don't know what the 'apply project directory' button does. Or 'refresh paths'
5. I asked if the system could build whatever was in the project directory 'source' folder, so that no cmakelists.txt file was needed in the project, because I thought it would be a pain. It's probably not a pain, and would help VSCode know where the headers etc are for intellisense etc. I don't really know or understand what this cmake 'template' is that is used to compile game projects. In any case, windows builds on linux don't work, they complain about not being able to find X11, which makes me think either the 'template' is wrong, or it doesn't know to use the linux toolchain file.
6. The editor startup should be to contact the backend, and also give you a select / create project panel to get started.
7. The build & run path is different to the preview path: obviously because of the 'hosted preview', but also the hosted preview doesn't seem to know anything about elapsed time / per frame timers, so it has no concept of 'dt'.
8. On linux, 'dist' builds almost work out of the box, but not quite. First, the binary doesn't know where to find dependencies and needs an LD_LIBRARY_PATH hint, and then there's a missing symlink. Feels like these c an both be solved.
9. The engine doesn't realise when a game has been closed by quitting its window, so keeps the old process id and you have to click 'stop' to clear it even though the game is already stopped.
