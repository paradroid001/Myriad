cmake_minimum_required(VERSION "3.18.4")
set(CMAKE_VERBOSE_MAKEFILE TRUE)
project("MyriadEngine" LANGUAGES CXX VERSION 0.0.1 DESCRIPTION "Myriad is a Game Engine")
#set(CMAKE_CXX_STANDARD 20)
#set(SPDLOG_USE_STD_FORMAT OFF)
#set(SPDLOG_FMT_EXTERNAL OFF)
#add_subdirectory(vendor/spdlog) #has its own cmakelists
#header only spdlog:
#target_include_directories("MyriadEngine" PUBLIC )

set(PLATFORM Desktop)

#We don't want raylib's examples built. This option is picked up by raylib's CMakeLists.txt
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/raylib)

#add_library(${PROJECT_NAME} SHARED) # Make a dynamic lib
#target_compile_definitions(${PROJECT_NAME} PRIVATE MYR_BUILD_DLL)
add_library(${PROJECT_NAME} STATIC) # Make a static lib

#set so version to major number of version, so myriad.so.1 -> myriad.so.1.0.0
#set(CMAKE_CXX_STANDARD 17)
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR} CMAKE_CXX_STANDARD 17)
if(CMAKE_SYSTEM_NAME STREQUAL Windows)
target_link_options(${PROJECT_NAME} PRIVATE -static-libgcc -lopengl32 -lgdi32 -lwinmm)
endif()
# I tried INTERFACE here instead of PUBLIC, and it didn't work.
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include/")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include/spdlog/")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/vendor/raylib/src/")

#define MY_SOURCES to be all my globbed cpp files.
file(GLOB_RECURSE MYRIAD_SOURCES CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cpp")
#add_library("MyriadEngine" "${MYRIAD_SOURCES}") #doing this with target_sources below

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/")
target_sources(${PROJECT_NAME} PRIVATE "${MYRIAD_SOURCES}")
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER src/myriad.h)

# since we're compiling the myriad library, it needs to know it's 'internal'
target_compile_definitions(${PROJECT_NAME} PRIVATE MYRIAD_INTERNAL)
target_compile_options(${PROJECT_NAME} PRIVATE -std=c++17 -Wall)

#
# TESTS
#
#find_package(Catch2 3 REQUIRED)
# Rather than find package, could also do:
add_subdirectory(vendor/catch2)
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/vendor/catch2/src")
# These tests can use the Catch2-provided main
add_executable(EngineTests tests/test.cpp src/io/Log.cpp)
target_include_directories(EngineTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/")
target_include_directories(EngineTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include/")
target_include_directories(EngineTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include/spdlog/")
target_include_directories(EngineTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/raylib/src/")
target_link_libraries(EngineTests PRIVATE Catch2::Catch2WithMain)
target_compile_options(EngineTests PRIVATE -std=c++17 -Wall)
