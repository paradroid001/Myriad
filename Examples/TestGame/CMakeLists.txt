project("TestGame" VERSION 0.0.1 DESCRIPTION "An example for the Myriad Game Engine")
cmake_minimum_required(VERSION "3.18.4")

#file(GLOB_RECURSE EXAMPLE_SOURCES CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE
#src/basic_logger.cpp
src/TestGameObjectManager.cpp
src/BounceBehaviour.cpp
src/TestMovementBehaviour.cpp
src/basic_example.cpp
)

#target_include_directories(${PROJECT_NAME} PUBLIC "../Engine/include/")
target_link_libraries(${PROJECT_NAME} MyriadEngine)
#target_link_libraries("MyriadTest" MyriadEngine)
target_compile_options(${PROJECT_NAME} PRIVATE -std=c++17 -Wall)
