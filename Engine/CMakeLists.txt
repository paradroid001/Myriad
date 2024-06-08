cmake_minimum_required(VERSION "3.18.4")

project("MyriadEngineProject")
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(vendor/spdlog) #has its own cmakelists

#set(PLATFORM=PLATFORM_DESKTOP)
#set(GRAPHICS=GRAPHICS_API_OPENGL_21)
add_subdirectory(vendor/raylib)

add_library("MyriadEngine")
# I tried INTERFACE here instead of PUBLIC, and it didn't work.
target_include_directories("MyriadEngine" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include/")
target_include_directories("MyriadEngine" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/vendor/raylib/src/")

# since we're compiling the myriad library, it needs to know it's 'internal'
add_compile_definitions(MYRIAD_INTERNAL)

#define MY_SOURCES to be all my globbed cpp files.
file(GLOB_RECURSE MYRIAD_SOURCES CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cpp")
#add_library("MyriadEngine" "${MYRIAD_SOURCES}") #doing this with target_sources below

target_include_directories("MyriadEngine" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/")
target_sources("MyriadEngine" PRIVATE "${MYRIAD_SOURCES}")
target_link_libraries("MyriadEngine" PRIVATE raylib)


