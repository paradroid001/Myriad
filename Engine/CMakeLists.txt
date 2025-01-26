cmake_minimum_required(VERSION "3.18.4")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE TRUE)
project("MyriadEngine" LANGUAGES CXX VERSION 0.0.1 DESCRIPTION "Myriad is a Game Engine")
### MYRIAD
#
# Make a dynamic lib
add_library(${PROJECT_NAME} SHARED)
#make sure dllimport/export is generated on windows
target_compile_definitions(${PROJECT_NAME} PRIVATE MYR_BUILD_DLL)
 # since we're compiling the myriad library, it needs to know it's 'internal'
target_compile_definitions(${PROJECT_NAME} PRIVATE MYRIAD_INTERNAL)
set(CMAKE_CXX_STANDARD 17)
target_compile_options(${PROJECT_NAME} PRIVATE -std=c++17 -Wall)
#set so version to major number of version, so myriad.so.1 -> myriad.so.1.0.0
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR} CMAKE_CXX_STANDARD 17)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_sources(${PROJECT_NAME} PRIVATE
    # CORE
    src/core/MyrApplication.cpp
    src/core/MyrGameApplication.cpp
    src/core/MyrEntity.cpp
    src/core/MyrComponent.cpp
    src/core/MyrEntityManager.cpp
    src/core/MyrProfiling.cpp
    src/core/MyrGameObject.cpp
    src/core/MyrGameEngine.cpp
    # IO
    src/io/MyrLogging.cpp
    src/io/LoggingProviderSpdLog.cpp

    # ASSETS
    src/asset/AssetManager.cpp
    src/asset/Font.cpp
    src/asset/Texture2D.cpp
    #if raylib..
    #normally you'd only compile this if MYRIAD_RENDERER == 1
    src/asset/FontProviderRaylib.cpp
    src/asset/TextureProviderRaylib.cpp
    src/gfx/RendererProviderRaylib.cpp

    # GFX
    src/gfx/WindowProviderRaylib.cpp
    src/gfx/Window.cpp
    src/gfx/Renderer.cpp

    # UTIL
    src/util/MyrRandom.cpp

    # GAME
    src/game/Transform.cpp
    src/game/GameObject.cpp
    #vendor/Remotery/lib/Remotery.c
)
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER include/myriad.h)
### Remotery (profiling)
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/vendor/Remotery/lib/")

### RAYLIB
#
#We don't want raylib's examples built. This option is picked up by raylib's CMakeLists.txt
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
#Build raylib as a shared lib?: We can't do this with zig c++ due to unsupported rpath-link.
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
# build raylib PIC: this should only be off if directly linking exe against static raylib (never)
set(WITH_PIC ON CACHE BOOL "" FORCE)

add_subdirectory(vendor/raylib)
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/vendor/raylib/src/")
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

### SPDLOG
#
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include/")

### JSON
#
add_subdirectory(vendor/json)
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/vendor/json/include/")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/vendor/json/include/nlohmann/")


### Tests
#
#compile tests
add_executable(EngineTests tests/basic_tests.cpp) # src/io/Log.cpp)
target_include_directories(EngineTests PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/vendor/catch2/src")
target_include_directories(EngineTests PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/")
target_include_directories(EngineTests PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
#compile catch2 code
add_subdirectory(vendor/catch2)
#link against Catch2, and against the Engine
target_link_libraries(EngineTests Catch2::Catch2WithMain ${PROJECT_NAME})
