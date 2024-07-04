cmake_minimum_required(VERSION "3.18.4")

project("MyriadEngineProject")

#set(CMAKE_CXX_STANDARD 20)
#set(SPDLOG_USE_STD_FORMAT OFF)
#set(SPDLOG_FMT_EXTERNAL OFF)
#add_subdirectory(vendor/spdlog) #has its own cmakelists
#header only spdlog:
#target_include_directories("MyriadEngine" PUBLIC )

set(PLATFORM Desktop)
#set(CMAKE_SYSTEM_NAME Windows)
#set(UNIX off)
#set(GLFW_BUILD_WAYLAND OFF)
set(GLFW_BUILD_X11 ON)

#We don't want raylib's examples built. This option is picked up by raylib's CMakeLists.txt
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/raylib)

set(CMAKE_CXX_STANDARD 17)

add_library("MyriadEngine")
# I tried INTERFACE here instead of PUBLIC, and it didn't work.
target_include_directories("MyriadEngine" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include/")
target_include_directories("MyriadEngine" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include/spdlog/")
target_include_directories("MyriadEngine" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/vendor/raylib/src/")

# since we're compiling the myriad library, it needs to know it's 'internal'
add_compile_definitions(MYRIAD_INTERNAL)

#define MY_SOURCES to be all my globbed cpp files.
file(GLOB_RECURSE MYRIAD_SOURCES CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cpp")
#add_library("MyriadEngine" "${MYRIAD_SOURCES}") #doing this with target_sources below

target_include_directories("MyriadEngine" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/")
target_sources("MyriadEngine" PRIVATE "${MYRIAD_SOURCES}")
target_link_libraries("MyriadEngine" PRIVATE raylib)
