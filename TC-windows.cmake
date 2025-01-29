set(CMAKE_VERBOSE_MAKEFILE ON)
# the name of the target operating system
set(CMAKE_SYSTEM_NAME Windows)
set(UNIX off)
set(GLFW_BUILD_X11 OFF)
set(GLFW_BUILD_WAYLAND OFF)
#If we turn this off, all the dllimport/export stuff is turned
#off. Will only work with static libs
add_compile_definitions(MYR_PLATFORM_WINDOWS)
