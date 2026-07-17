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

The editor now supports a TCP build bridge instead of relying on local cmake when you enable socket mode. The client path lives in Editor/src/main.cpp, and the container-side listener is docker/build_bridge_server.py. The bridge also applies the preset environment variables from CMakeKits.json before it runs cmake, and I forwarded the bridge port in .devcontainer/devcontainer.json.

To use it from an external editor, run the bridge inside the container with python3 build_bridge_server.py --host 0.0.0.0 --port 55333, then set MYRIAD_BUILD_MODE=socket or add the socket settings shown in README.md.

For WSL or native Linux, keep the bridge host pointed at the local forwarded port unless your Docker setup exposes it differently.

Example launch command from inside the container shell:

`python3 /workspaces/Myriad/docker/build_bridge_server.py --host 0.0.0.0 --port 55333`

## API documentation (Doxygen)

The repository now includes a root Doxygen config at `Doxyfile` for the editor module (`Editor/src`).

Install Doxygen (Ubuntu/Debian):

`sudo apt-get update && sudo apt-get install -y doxygen`

Generate docs from the repository root:

`doxygen Doxyfile`

Or use the helper script:

`./scripts/generate-docs.sh`

Generate and open the HTML docs in a browser:

`./scripts/generate-docs.sh --open`

Generated HTML entry point:

`docs/doxygen/html/index.html`
