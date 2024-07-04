cmake_minimum_required(VERSION "3.18.4")
project("Myriad")
#set(CMAKE_TOOLCHAIN_FILE "TC-windows.cmake")

# the name of the target operating system
#set(CMAKE_SYSTEM_NAME Windows)

# which compilers to use for C and C++
#set(CMAKE_C_COMPILER:FILEPATH="/home/myriad/zig/zig_c_windows.sh")
#set(CMAKE_CXX_COMPILER:FILEPATH="/home/myriad/zig/zig_cpp_windows.sh")


add_subdirectory("Engine")
add_subdirectory("Test")
