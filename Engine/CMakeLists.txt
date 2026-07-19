cmake_minimum_required(VERSION "3.25")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE TRUE)
project(MyriadEngine LANGUAGES CXX VERSION 0.0.1 DESCRIPTION "Myriad is a Game Engine")

# Make a dynamic lib (this will not be the case for EMSCRIPTEN)
add_library(${PROJECT_NAME} SHARED)
myriad_apply_mingw_libgcc(${PROJECT_NAME})

if(APPLE)
    set(MYRIAD_ENGINE_INSTALL_RPATH "@loader_path")
elseif(UNIX)
    set(MYRIAD_ENGINE_INSTALL_RPATH "$ORIGIN")
else()
    set(MYRIAD_ENGINE_INSTALL_RPATH "")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    INSTALL_RPATH "${MYRIAD_ENGINE_INSTALL_RPATH}"
)
#make sure dllimport/export is generated on windows
target_compile_definitions(${PROJECT_NAME} PRIVATE MYR_BUILD_DLL)
# since we're compiling the myriad library, it needs to know it's 'internal'
target_compile_definitions(${PROJECT_NAME} PRIVATE MYRIAD_INTERNAL)
set(CMAKE_CXX_STANDARD 17)

# compile with all warnings, and warnings as errors. No rtti or exceptions, as they are not needed and add overhead.
target_compile_options(${PROJECT_NAME} PRIVATE -std=c++17;-Wall;-Werror;-fno-rtti;-fno-exceptions)

# set so version to major number of version, so myriad.so.1 -> myriad.so.1.0.0
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR} CMAKE_CXX_STANDARD 17)

set(MYRIAD_ENGINE_INSTALL_COMPONENT "MyriadEngine")

# Public includes
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
# Private includes
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/")

target_sources(${PROJECT_NAME} PRIVATE
    # CORE
    src/core/MyrApplication.cpp
    src/core/GameEngine.cpp
    src/core/MyrGameApplication.cpp
    src/core/MyrRandom.cpp
    # GFX
    src/gfx/Renderer.cpp
    src/gfx/RendererProviderRaylib.cpp
    src/gfx/Window.cpp
    src/gfx/Window.h
    src/gfx/WindowProviderRaylib.cpp
    # IO
    src/io/ILogger.cpp
    src/io/BasicLogger.cpp
    # ASSET
    src/asset/MyrAssetManager.cpp
    #src/asset/MyrTexture2D.cpp
    src/asset/Texture2DProviderRaylib.cpp
    #src/asset/MyrFont.cpp
    src/asset/FontProviderRaylib.cpp

)

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER
 include/myriad.h
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION "${MYRIAD_INSTALL_BIN_DIR}"
    COMPONENT ${MYRIAD_ENGINE_INSTALL_COMPONENT}
    LIBRARY DESTINATION "${MYRIAD_INSTALL_LIB_DIR}"
    COMPONENT ${MYRIAD_ENGINE_INSTALL_COMPONENT}
    ARCHIVE DESTINATION "${MYRIAD_INSTALL_LIB_DIR}"
    COMPONENT ${MYRIAD_ENGINE_INSTALL_COMPONENT}
    PUBLIC_HEADER DESTINATION "${MYRIAD_INSTALL_INCLUDE_DIR}"
    COMPONENT ${MYRIAD_ENGINE_INSTALL_COMPONENT}
)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
    DESTINATION "${MYRIAD_INSTALL_INCLUDE_DIR}"
    COMPONENT ${MYRIAD_ENGINE_INSTALL_COMPONENT}
    FILES_MATCHING
    PATTERN "*.h"
    PATTERN "*.hpp"
)

# Copy resolved runtime shared-library dependencies next to the installed shared object.
if(NOT MINGW)
    install(CODE "
        file(GET_RUNTIME_DEPENDENCIES
            LIBRARIES \"$<TARGET_FILE:MyriadEngine>\"
            RESOLVED_DEPENDENCIES_VAR _myriad_resolved_deps
            UNRESOLVED_DEPENDENCIES_VAR _myriad_unresolved_deps
        )

        if(_myriad_unresolved_deps)
            message(WARNING \"Unresolved runtime deps for MyriadEngine: \${_myriad_unresolved_deps}\")
        endif()

        if(_myriad_resolved_deps)
            file(INSTALL
                DESTINATION \"${CMAKE_INSTALL_PREFIX}/${MYRIAD_INSTALL_LIB_DIR}\"
                TYPE SHARED_LIBRARY
                FOLLOW_SYMLINK_CHAIN
                FILES \${_myriad_resolved_deps}
            )
        endif()
    " COMPONENT ${MYRIAD_ENGINE_INSTALL_COMPONENT})
endif()

enable_testing()

# Catch2 unit tests for Engine
if(NOT TARGET Catch2::Catch2WithMain)
    add_subdirectory("${CMAKE_SOURCE_DIR}/shared/vendor/catch2" "${CMAKE_BINARY_DIR}/catch2" EXCLUDE_FROM_ALL)
endif()

# On windows, embed libstdc++ and libgcc into the dll, so that users don't need to worry about installing them. This is not needed for zig c++ builds, as zig statically links the c++ standard library by default.
#if(MINGW)
#    target_link_options(${PROJECT_NAME} PRIVATE "-static-libgcc" "-static-libstdc++" "-static")
#endif()

### RAYLIB
#
#We don't want raylib's examples built. This option is picked up by raylib's CMakeLists.txt
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
#Build raylib as a shared lib?: We can't do this with zig c++ due to unsupported rpath-link.
if(MINGW)
    set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
    set(OPENGL_VERSION "OFF" CACHE STRING "raylib OpenGL context version" FORCE)
else()
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
endif()
# build raylib PIC: this should only be off if directly linking exe against static raylib (never)
set(WITH_PIC ON CACHE BOOL "" FORCE)

add_subdirectory(vendor/raylib)
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/raylib/src/")
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

if(MINGW)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
endif()

### TRACY PROFILER
#
# Enable Tracy profiling library
option(TRACY_ENABLE ON)
option(TRACY_ON_DEMAND ON)
# Build statically
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
# Could potentially do this instead:
# set_target_properties(TracyClient PROPERTIES POSITION_INDEPENDENT_CODE ON)
add_subdirectory(vendor/tracy)
target_link_libraries(${PROJECT_NAME} PRIVATE Tracy::TracyClient)

add_subdirectory(tests)
