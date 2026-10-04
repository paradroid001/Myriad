cmake_minimum_required(VERSION "3.25")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE TRUE)
project(MyriadEngine LANGUAGES CXX VERSION 0.0.1 DESCRIPTION "Myriad is a Game Engine")
set(CMAKE_CXX_STANDARD 17)

# Build MyriadEngine as a shared library so examples/games can link it without
# recompiling it. Emscripten has no usable load-time/runtime dynamic linking story
# for this project's needs, so it gets a static library instead (see the Minimal
# example's CMakeLists.txt, which mirrors this choice for its own library target).
if(EMSCRIPTEN)
    add_library(${PROJECT_NAME} STATIC)
else()
    add_library(${PROJECT_NAME} SHARED)
endif()

# MYR_BUILD_DLL: selects dllexport (vs. dllimport) for MYR_API symbols on Windows; no-op elsewhere.
# MYRIAD_INTERNAL: exposes engine-internal declarations that client code shouldn't rely on.
target_compile_definitions(${PROJECT_NAME} PRIVATE MYR_BUILD_DLL MYRIAD_INTERNAL)

# compile with all warnings, and warnings as errors. No rtti or exceptions, as they are not needed and add overhead.
target_compile_options(${PROJECT_NAME} PRIVATE -std=c++17;-Wall;-Werror;-fno-rtti;-fno-exceptions)

# So the shared object is versioned as libmyriad.so.<major>, e.g. libmyriad.so.0 -> libmyriad.so.0.0.1.
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

set(MYRIAD_ENGINE_INSTALL_COMPONENT "MyriadEngine")

# Public includes
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
# Private includes
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/")


target_sources(${PROJECT_NAME} PRIVATE
   # CORE
   src/core/Application.cpp
   src/core/GameApplication.cpp
   src/core/GameEngine.cpp
   src/core/Random.cpp
   src/core/Renderer.cpp
   src/core/Window.cpp
   # IO
   src/io/ILogger.cpp
   src/io/BasicLogger.cpp
   # Providers
   src/providers/raylib/RenderProviderRaylib.cpp
   src/providers/raylib/WindowProviderRaylib.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    PUBLIC_HEADER include/myriad.h
)

# Installation rules for the MyriadEngine library.
# Look for dependent libraries next to this one at runtime, rather than at a fixed
# absolute path, so an install can be relocated freely.
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
# (Not applicable to MINGW's static-everything builds, nor to Emscripten where MyriadEngine is static.)
if(NOT MINGW AND NOT EMSCRIPTEN)
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

myriad_apply_mingw_libgcc(${PROJECT_NAME})

# Windows: link libraries Remotery needs (not yet integrated; condition is currently
# never true since CMAKE_SYSTEM_NAME should be compared with STREQUAL, not EQUAL).
if(CMAKE_SYSTEM_NAME EQUAL Windows)
    target_link_options(${PROJECT_NAME} -lwinmm -libws2_32)
endif()
# On windows, embed libstdc++ and libgcc into the dll, so that users don't need to worry about installing them. This is not needed for zig c++ builds, as zig statically links the c++ standard library by default.
#if(MINGW)
#    target_link_options(${PROJECT_NAME} PRIVATE "-static-libgcc" "-static-libstdc++" "-static")
#endif()

### RAYLIB
#
#We don't want raylib's examples built. This option is picked up by raylib's CMakeLists.txt
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
# Build raylib (and its vendored GLFW) statically: zig c++ doesn't support rpath-link,
# which a shared raylib build would need.
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
# build raylib PIC: this should only be off if directly linking exe against static raylib (never)
set(WITH_PIC ON CACHE BOOL "" FORCE)

add_subdirectory(vendor/raylib)
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/vendor/raylib/src/")
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)


### Tests
#
#enable_testing()
# Catch2 unit tests for Engine
#if(NOT TARGET Catch2::Catch2WithMain)
#    add_subdirectory("${CMAKE_SOURCE_DIR}/shared/vendor/catch2" "${CMAKE_BINARY_DIR}/catch2" EXCLUDE_FROM_ALL)
#endif()

#compile tests
#add_executable(EngineTests tests/basic_tests.cpp) # src/io/Log.cpp)
#target_include_directories(EngineTests PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/vendor/catch2/src")
#target_include_directories(EngineTests PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/")
#target_include_directories(EngineTests PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
#compile catch2 code
#add_subdirectory(vendor/catch2)
#link against Catch2, and against the Engine
#target_link_libraries(EngineTests Catch2::Catch2WithMain ${PROJECT_NAME})
#add_subdirectory(tests)
