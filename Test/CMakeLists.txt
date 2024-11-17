project("MyriadTest" VERSION 0.0.1 DESCRIPTION "A test for the Myriad Game Engine")
cmake_minimum_required(VERSION "3.18.4")
set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CXX_STANDARD 17)

if(CMAKE_SYSTEM_NAME EQUAL Windows)
target_link_options(-static-libgcc -lopengl32 -lgdi32 -lwinmm)
endif()

if (EMSCRIPTEN)
    #set(CMAKE_LINK_LIBRARY_FLAG "${CMAKE_LINK_LIBRARY_FLAG} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY -s GL_ENABLE_GET_PROC_ADDRESS=1")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file ${CMAKE_CURRENT_LIST_DIR}/../res/@res/ -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY -s GL_ENABLE_GET_PROC_ADDRESS=1")
    set(CMAKE_EXECUTABLE_SUFFIX ".html") # This line is used to set your executable to build with the emscripten html template so that you can directly open it.
endif ()

#set_target_properties("MyriadTest" PROPERTIES CMAKE_CXX_STANDARD 17)
file(GLOB_RECURSE TEST_SOURCES CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_executable(MyriadTest "${TEST_SOURCES}")
target_include_directories(${PROJECT_NAME} PUBLIC "../Engine/src")
target_link_libraries(MyriadTest PRIVATE MyriadEngine)
#target_link_libraries("MyriadTest" MyriadEngine)
target_compile_options(${PROJECT_NAME} PRIVATE -std=c++17 -Wall)
