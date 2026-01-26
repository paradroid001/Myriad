cmake_minimum_required(VERSION "3.18.4")
project("Myriad")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CXX_STANDARD 17)


# The libws2_32 is needed for Remotery
if(CMAKE_SYSTEM_NAME EQUAL Windows)
target_link_options(-static-libgcc -lopengl32 -lgdi32 -lwinmm -libws2_32)
endif()
set(EMSCRIPTEN_PTHREADS_COMPILER_FLAGS "")
#set(EMSCRIPTEN_PTHREADS_COMPILER_FLAGS "-pthread")
set(EMSCRIPTEN_PTHREADS_LINKER_FLAGS "")
#set(EMSCRIPTEN_PTHREADS_LINKER_FLAGS "${EMSCRIPTEN_PTHREADS_COMPILER_FLAGS} -sPROXY_TO_PTHREAD")
set(EMSCRIPTEN_PRELOAD_RESOURCES_LINKER_FLAGS "--preload-file ${CMAKE_CURRENT_LIST_DIR}/res/@res/")
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
add_subdirectory("Engine")
add_subdirectory("Editor")
add_subdirectory("Examples/TestGame")
add_subdirectory("Examples/Terminal")
add_subdirectory("Examples/TestEvents")
