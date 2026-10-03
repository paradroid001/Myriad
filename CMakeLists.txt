cmake_minimum_required(VERSION "3.18.4")
project("Myriad")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CXX_STANDARD 17)

# Use /workspaces/Myriad/dist as the default install root unless the user overrides it.
# An override might point to say, a bind mounted directory outside the container
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/workspaces/Myriad/dist" CACHE PATH "Install path prefix" FORCE)
endif()

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

if(CMAKE_CONFIGURATION_TYPES)
    set(MYRIAD_INSTALL_CONFIG "$<CONFIG>")
else()
    set(MYRIAD_INSTALL_CONFIG "${CMAKE_BUILD_TYPE}")
endif()

# Installing builds from different kits (host gcc, mingw, emscripten, ...) into the same
# prefix would overwrite each other's outputs, so each install is namespaced under a
# "toolkit" name inferred from the build directory layout. Override with -DMYRIAD_BUILD_TOOLKIT=<name>.
set(MYRIAD_BUILD_TOOLKIT "" CACHE STRING "Build toolkit name used in install paths")
if(NOT MYRIAD_BUILD_TOOLKIT)
    get_filename_component(MYRIAD_BINARY_DIR_NAME "${CMAKE_BINARY_DIR}" NAME)
    get_filename_component(MYRIAD_BINARY_PARENT_DIR "${CMAKE_BINARY_DIR}" DIRECTORY)
    get_filename_component(MYRIAD_BINARY_PARENT_NAME "${MYRIAD_BINARY_PARENT_DIR}" NAME)
    string(TOLOWER "${MYRIAD_BINARY_DIR_NAME}" MYRIAD_BINARY_DIR_NAME_LOWER)
    string(TOLOWER "${CMAKE_BUILD_TYPE}" MYRIAD_BUILD_TYPE_LOWER)

    # Used whenever the build directory name isn't informative on its own.
    set(MYRIAD_COMPILER_FINGERPRINT "${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_NAME}")

    if(NOT CMAKE_CONFIGURATION_TYPES AND MYRIAD_BINARY_DIR_NAME_LOWER STREQUAL MYRIAD_BUILD_TYPE_LOWER AND NOT MYRIAD_BINARY_PARENT_NAME STREQUAL "")
        # Layout is <toolkit>/<Config>/ (e.g. build/G++.../Debug): the parent dir names the toolkit.
        set(MYRIAD_INFERRED_BUILD_TOOLKIT "${MYRIAD_BINARY_PARENT_NAME}")
    elseif(NOT MYRIAD_BINARY_DIR_NAME STREQUAL "")
        # Layout is just <toolkit>/: the binary dir itself names the toolkit.
        set(MYRIAD_INFERRED_BUILD_TOOLKIT "${MYRIAD_BINARY_DIR_NAME}")
    else()
        set(MYRIAD_INFERRED_BUILD_TOOLKIT "${MYRIAD_COMPILER_FINGERPRINT}")
    endif()

    # "build" itself isn't a meaningful toolkit name (e.g. configuring straight into
    # <root>/build with no kit subfolder) - fall back to the compiler fingerprint.
    if(MYRIAD_INFERRED_BUILD_TOOLKIT STREQUAL "build")
        set(MYRIAD_INFERRED_BUILD_TOOLKIT "${MYRIAD_COMPILER_FINGERPRINT}")
    endif()

    set(MYRIAD_BUILD_TOOLKIT "${MYRIAD_INFERRED_BUILD_TOOLKIT}" CACHE STRING "Build toolkit name used in install paths" FORCE)
endif()

set(MYRIAD_INSTALL_ROOT "myriad")
set(MYRIAD_INSTALL_INCLUDE_DIR "${MYRIAD_INSTALL_ROOT}/include")
set(MYRIAD_INSTALL_SHARE_DIR "${MYRIAD_INSTALL_ROOT}/share")
set(MYRIAD_INSTALL_BINARY_ROOT "${MYRIAD_INSTALL_ROOT}/${MYRIAD_BUILD_TOOLKIT}/${MYRIAD_INSTALL_CONFIG}")
set(MYRIAD_INSTALL_BIN_DIR "${MYRIAD_INSTALL_BINARY_ROOT}/bin")
set(MYRIAD_INSTALL_LIB_DIR "${MYRIAD_INSTALL_BINARY_ROOT}/lib")


# Planned, not yet wired up: linking against Remotery on Windows needs -lwinmm -lws2_32.
# if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
# target_link_options(-static-libgcc -lopengl32 -lgdi32 -lwinmm -lws2_32)
# endif()

# Pthreads are disabled for now; set these (and enable -sPROXY_TO_PTHREAD) to turn them on.
set(EMSCRIPTEN_PTHREADS_COMPILER_FLAGS "")
set(EMSCRIPTEN_PTHREADS_LINKER_FLAGS "")
# Bundle shared/res/ into the Emscripten virtual filesystem at /shared/res.
set(EMSCRIPTEN_PRELOAD_RESOURCES_LINKER_FLAGS "--preload-file ${CMAKE_CURRENT_LIST_DIR}/shared/res/@shared/res/")
# USE_GLFW=3: use Emscripten's GLFW3 port, which raylib's Web backend builds against.
# ASSERTIONS=1: keep Emscripten's runtime checks enabled to make failures easier to diagnose.
# ASYNCIFY: lets blocking-style C++ code yield to the browser's event loop.
# GL_ENABLE_GET_PROC_ADDRESS=1: needed by raylib's GL function loader.
set(EMSCRIPTEN_OTHER_LINKER_FLAGS "-s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY -s GL_ENABLE_GET_PROC_ADDRESS=1")

if(EMSCRIPTEN)
    string(APPEND CMAKE_C_FLAGS " ${EMSCRIPTEN_PTHREADS_COMPILER_FLAGS}")
    string(APPEND CMAKE_CXX_FLAGS " ${EMSCRIPTEN_PTHREADS_COMPILER_FLAGS}")
    string(APPEND CMAKE_EXE_LINKER_FLAGS " ${EMSCRIPTEN_PRELOAD_RESOURCES_LINKER_FLAGS} ${EMSCRIPTEN_OTHER_LINKER_FLAGS} ${EMSCRIPTEN_PTHREADS_LINKER_FLAGS}")
    # Produce an .html launcher (alongside .js/.wasm) so the build can be opened directly in a browser.
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

# raylib picks its backend from PLATFORM (Desktop: GLFW+OpenGL, Web: Emscripten's GLFW port).
# CMAKE_SYSTEM_NAME is set to "Emscripten" by its toolchain file.
if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    set(PLATFORM "Web")
else()
    set(PLATFORM "Desktop")
endif()

option(MYRIAD_MINGW_LIBGCC_SHARED "Use shared libgcc on MinGW targets (OFF uses static libgcc)." OFF)

# Centralizes the MinGW libgcc/libstdc++ linkage choice so every target applies it consistently.
function(myriad_apply_mingw_libgcc target_name)
    if(MINGW)
        if(MYRIAD_MINGW_LIBGCC_SHARED)
            target_link_options(${target_name} PRIVATE -shared-libgcc -shared-libstdc++)
        else()
            target_link_options(${target_name} PRIVATE -static-libgcc -static-libstdc++)
        endif()
    endif()
endfunction()

add_subdirectory("Engine")
# Other targets, currently disabled while under construction:
#add_subdirectory("Editor")
#add_subdirectory("Examples/TestGame")
#add_subdirectory("Examples/Terminal")
add_subdirectory("Examples/Minimal")
