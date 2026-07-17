project("TestECS" VERSION 0.0.1 DESCRIPTION "An example for the Myriad Game Engine")
cmake_minimum_required(VERSION "3.18.4")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(COMPILE_FLAGS -std=c++17 -Wall -Werror -fno-rtti -fno-exceptions)
# Logical names
set(LIBRARY_NAME lib${PROJECT_NAME})
set(EXECUTABLE_NAME ${PROJECT_NAME})

#Build TestGame as a Library
add_library(${LIBRARY_NAME} SHARED)
myriad_apply_mingw_libgcc(${LIBRARY_NAME})
target_compile_options(${LIBRARY_NAME} PRIVATE ${COMPILE_FLAGS})
set_target_properties(${LIBRARY_NAME} PROPERTIES
    OUTPUT_NAME ${PROJECT_NAME}
)
target_sources(${LIBRARY_NAME} PRIVATE
src/ECS3.cpp
src/TestECS.cpp
src/HostedFactory.cpp
)
target_include_directories(${LIBRARY_NAME} PUBLIC "../Engine/include/")
#Link against the Myriad Engine
target_link_libraries(${LIBRARY_NAME} MyriadEngine)
#if(MINGW)
#    target_link_options(${LIBRARY_NAME} PRIVATE "-static-libgcc" "-static-libstdc++" "-static")
#endif()


# Build the Executable Stub which launches it.
add_executable(${EXECUTABLE_NAME})
myriad_apply_mingw_libgcc(${EXECUTABLE_NAME})
# compile with all warnings, and warnings as errors. No rtti or exceptions, as they are not needed and add overhead.
target_compile_options(${EXECUTABLE_NAME} PRIVATE ${COMPILE_FLAGS})
target_sources(${EXECUTABLE_NAME} PRIVATE
src/main.cpp
)

target_include_directories(${EXECUTABLE_NAME} PUBLIC "../Engine/include/")
target_link_libraries(${EXECUTABLE_NAME} MyriadEngine ${LIBRARY_NAME})

if(WIN32)
# Once Built, copy the dll to the executable output directory,
# so that it can be found when running the exe.
# Automatically locate runtime dependencies after the target is built
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND_EXPAND_LISTS

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
)
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
