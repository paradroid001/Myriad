set(CMAKE_VERBOSE_MAKEFILE ON)
# the name of the target operating system
set(CMAKE_SYSTEM_NAME Windows)
set(UNIX off)
set(GLFW_BUILD_X11 OFF)
set(GLFW_BUILD_WAYLAND OFF)
# Allow running Windows-built executables from WSL by providing a host-side emulator.
# This doesn't seem to work...
#set(CMAKE_CROSSCOMPILING_EMULATOR "${CMAKE_CURRENT_LIST_DIR}/run-windows-executable.sh" CACHE PATH "Emulator for running Windows target executables from WSL")

#If we turn this off, all the dllimport/export stuff is turned
#off. Will only work with static libs
add_compile_definitions(MYR_PLATFORM_WINDOWS)

# Runtime linkage is configured per target in project CMakeLists files.
# Keep toolchain flags neutral to avoid hidden global coupling.
set(CMAKE_CXX_FLAGS "" CACHE STRING "c++ flags" FORCE)
set(CMAKE_C_FLAGS "" CACHE STRING "c flags" FORCE)
