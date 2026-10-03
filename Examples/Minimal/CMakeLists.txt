project("MinimalExample" VERSION 0.0.1 DESCRIPTION "An example for the Myriad Game Engine")
cmake_minimum_required(VERSION "3.18.4")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(MYRIAD_MINIMAL_COMPILE_OPTIONS -std=c++17 -Wall -Werror -fno-rtti -fno-exceptions)
# Logical names
set(LIBRARY_NAME lib${PROJECT_NAME})
set(EXECUTABLE_NAME ${PROJECT_NAME})

# The example's game/app code lives in its own library (separate from the stub exe
# below), mirroring how a real game built on Myriad would be structured and hosted.
# Emscripten has no usable load-time/runtime dynamic linking story for this project's
# needs, so it gets a static library there instead (see Engine/CMakeLists.txt).
if(EMSCRIPTEN)
    add_library(${LIBRARY_NAME} STATIC)
else()
    add_library(${LIBRARY_NAME} SHARED)
endif()
myriad_apply_mingw_libgcc(${LIBRARY_NAME})
target_compile_options(${LIBRARY_NAME} PRIVATE ${MYRIAD_MINIMAL_COMPILE_OPTIONS})
# Reserved for distinguishing a library-hosted entry point from the stub exe's entry
# point below; not yet read anywhere.
target_compile_definitions(${LIBRARY_NAME} PRIVATE MYRIAD_HOSTED_ENTRYPOINT)
set_target_properties(${LIBRARY_NAME} PROPERTIES
    OUTPUT_NAME ${PROJECT_NAME}
)
target_sources(${LIBRARY_NAME} PRIVATE
    src/Minimal.cpp
)

target_include_directories(${LIBRARY_NAME} PUBLIC "../Engine/include/")
#Link against the Myriad Engine
target_link_libraries(${LIBRARY_NAME} MyriadEngine)

# Build the executable stub which launches it.
add_executable(${EXECUTABLE_NAME})
myriad_apply_mingw_libgcc(${EXECUTABLE_NAME})
# compile with all warnings, and warnings as errors. No rtti or exceptions, as they are not needed and add overhead.
target_compile_options(${EXECUTABLE_NAME} PRIVATE ${MYRIAD_MINIMAL_COMPILE_OPTIONS})
target_sources(${EXECUTABLE_NAME} PRIVATE
    src/main.cpp
)

# Stub needs to know about Myriad public types
target_include_directories(${EXECUTABLE_NAME} PUBLIC "../Engine/include/")
# Linked against MyriadEngine directly (not just transitively via LIBRARY_NAME) because
# that link is PRIVATE, so the engine's public include dirs wouldn't otherwise propagate here.
target_link_libraries(${EXECUTABLE_NAME} MyriadEngine ${LIBRARY_NAME})
if(WIN32)
    # Once built, copy MyriadEngine's (and any other) runtime DLLs next to the stub exe,
    # so it can find them without being installed.
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND_EXPAND_LISTS
    )

    # This copies dlls, but they are huge. For now, when
    # building with mingw, we will statically link.
    # This is done in the toolchain file, i.e doing the following,
    # but for all MINGW compilation.
    #   if(MINGW)
    #       target_link_options(${EXECUTABLE_NAME} PRIVATE "-static-libgcc" "-static-libstdc++" "-static")
    #   endif()

    #if(MINGW)
    #    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    #        COMMAND ${CMAKE_COMMAND} -E copy_if_different
    #        /usr/lib/gcc/x86_64-w64-mingw32/10-win32/libstdc++-6.dll
    #        /usr/lib/gcc/x86_64-w64-mingw32/10-win32/libgcc_s_seh-1.dll
    #        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    #        COMMAND_EXPAND_LISTS
    #    )
    #endif()
elseif(UNIX)
    # On linux, copy the engine lib to support running the
    # executable from the build directory without installing the engine.
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:MyriadEngine>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND_EXPAND_LISTS
    )
endif()
