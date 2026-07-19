cmake_minimum_required(VERSION "3.18.4")
project("Myriad")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CXX_STANDARD 17)

# Use /workspaces/dist as the default install root unless the user overrides it.
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/workspaces/dist" CACHE PATH "Install path prefix" FORCE)
endif()

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

if(CMAKE_CONFIGURATION_TYPES)
    set(MYRIAD_INSTALL_CONFIG "$<CONFIG>")
else()
    set(MYRIAD_INSTALL_CONFIG "${CMAKE_BUILD_TYPE}")
endif()

set(MYRIAD_BUILD_TOOLKIT "" CACHE STRING "Build toolkit name used in install paths")
if(NOT MYRIAD_BUILD_TOOLKIT)
    get_filename_component(MYRIAD_BINARY_DIR_NAME "${CMAKE_BINARY_DIR}" NAME)
    get_filename_component(MYRIAD_BINARY_PARENT_DIR "${CMAKE_BINARY_DIR}" DIRECTORY)
    get_filename_component(MYRIAD_BINARY_PARENT_NAME "${MYRIAD_BINARY_PARENT_DIR}" NAME)
    string(TOLOWER "${MYRIAD_BINARY_DIR_NAME}" MYRIAD_BINARY_DIR_NAME_LOWER)
    string(TOLOWER "${CMAKE_BUILD_TYPE}" MYRIAD_BUILD_TYPE_LOWER)

    if(NOT CMAKE_CONFIGURATION_TYPES AND MYRIAD_BINARY_DIR_NAME_LOWER STREQUAL MYRIAD_BUILD_TYPE_LOWER AND NOT MYRIAD_BINARY_PARENT_NAME STREQUAL "")
        set(MYRIAD_INFERRED_BUILD_TOOLKIT "${MYRIAD_BINARY_PARENT_NAME}")
    elseif(NOT MYRIAD_BINARY_DIR_NAME STREQUAL "")
        set(MYRIAD_INFERRED_BUILD_TOOLKIT "${MYRIAD_BINARY_DIR_NAME}")
    else()
        set(MYRIAD_INFERRED_BUILD_TOOLKIT "${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_NAME}")
    endif()

    if(MYRIAD_INFERRED_BUILD_TOOLKIT STREQUAL "build")
        set(MYRIAD_INFERRED_BUILD_TOOLKIT "${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_NAME}")
    endif()

    set(MYRIAD_BUILD_TOOLKIT "${MYRIAD_INFERRED_BUILD_TOOLKIT}" CACHE STRING "Build toolkit name used in install paths" FORCE)
endif()

set(MYRIAD_INSTALL_ROOT "myriad")
set(MYRIAD_INSTALL_INCLUDE_DIR "${MYRIAD_INSTALL_ROOT}/include")
set(MYRIAD_INSTALL_SHARE_DIR "${MYRIAD_INSTALL_ROOT}/share")
set(MYRIAD_INSTALL_BINARY_ROOT "${MYRIAD_INSTALL_ROOT}/${MYRIAD_BUILD_TOOLKIT}/${MYRIAD_INSTALL_CONFIG}")
set(MYRIAD_INSTALL_BIN_DIR "${MYRIAD_INSTALL_BINARY_ROOT}/bin")
set(MYRIAD_INSTALL_LIB_DIR "${MYRIAD_INSTALL_BINARY_ROOT}/lib")

set(EMSCRIPTEN_PTHREADS_COMPILER_FLAGS "")
#set(EMSCRIPTEN_PTHREADS_COMPILER_FLAGS "-pthread")
set(EMSCRIPTEN_PTHREADS_LINKER_FLAGS "")
#set(EMSCRIPTEN_PTHREADS_LINKER_FLAGS "${EMSCRIPTEN_PTHREADS_COMPILER_FLAGS} -sPROXY_TO_PTHREAD")
set(EMSCRIPTEN_PRELOAD_RESOURCES_LINKER_FLAGS "--preload-file ${CMAKE_CURRENT_LIST_DIR}/shared/res/@shared/res/")
set(EMSCRIPTEN_OTHER_LINKER_FLAGS "-s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY -s GL_ENABLE_GET_PROC_ADDRESS=1")

if(EMSCRIPTEN)
    string(APPEND CMAKE_C_FLAGS " ${EMSCRIPTEN_PTHREADS_COMPILER_FLAGS}")
    string(APPEND CMAKE_CXX_FLAGS " ${EMSCRIPTEN_PTHREADS_COMPILER_FLAGS}")
    string(APPEND CMAKE_EXE_LINKER_FLAGS " ${EMSCRIPTEN_PRELOAD_RESOURCES_LINKER_FLAGS} ${EMSCRIPTEN_OTHER_LINKER_FLAGS} ${EMSCRIPTEN_PTHREADS_LINKER_FLAGS}")
    set(CMAKE_EXECUTABLE_SUFFIX ".html") # This line is used to set your executable to build with the emscripten html template so that you can directly open it.
endif()

#Platform will have been set by kit?
if(CMAKE_SYSTEM_NAME STREQUAL Emscripten) #set by emscripten build chain
set(PLATFORM Web) # CACHE STRING "Platform to build for")
else()
set(PLATFORM Desktop) # CACHE STRING "Platform to build for")
endif()

option(MYRIAD_MINGW_LIBGCC_SHARED "Use shared libgcc on MinGW targets (OFF uses static libgcc)." OFF)

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
#add_subdirectory("Examples/TestGame")
add_subdirectory("Examples/TestECS")
add_subdirectory("Editor")
# Tracy Profiler Server
#set(LEGACY ON)
#add_subdirectory("shared/vendor/tracy/profiler")
