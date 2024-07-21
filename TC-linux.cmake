# the name of the target operating system
set(CMAKE_SYSTEM_NAME Linux)
set(UNIX on)
set(GLFW_BUILD_X11 ON)
set(GLFW_BUILD_WAYLAND OFF)

#set(CMAKE_BUILD_TYPE "Debug")
# "Release" "MinSizeRel" "RelWithDebInfo")

# which compilers to use for C and C++

#set(CMAKE_C_COMPILER "\'/home/myriad/zig/zig cc\'")
#set(CMAKE_CXX_COMPILER "\'/home/myriad/zig/zig c++\'")

#set(CMAKE_C_COMPILER   "zigcc")
#set(CMAKE_CXX_COMPILER "zigcpp")


# where is the target environment located
#set(CMAKE_FIND_ROOT_PATH  /home/myriad/zig)

# adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment
#set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# search headers and libraries in the target environment
#set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
