cmake_minimum_required(VERSION "3.18.4")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE TRUE)
project("MyriadEditor" LANGUAGES CXX VERSION 0.0.1 DESCRIPTION "Editor for Myriad Game Engine")
set(CMAKE_CXX_STANDARD 17)
add_executable(${PROJECT_NAME})
#target_compile_options(${PROJECT_NAME} PRIVATE -std=c++17 -Wall)
#set_target_properties(${PROJECT_NAME} PROPERTIES ${PROJECT_VERSION_MAJOR} CMAKE_CXX_STANDARD 17)

#link against myriad:
target_link_libraries(${PROJECT_NAME} MyriadEngine)
# had to add this line or it wouldn't link on windows
target_link_libraries(${PROJECT_NAME} raylib)


#Imgui Includes
set(IMGUI_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui)
set(RLIMGUI_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/rlImGui)
target_include_directories(${PROJECT_NAME} PUBLIC ${IMGUI_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${RLIMGUI_SOURCE_DIR})
#This proj includes
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
#Src
target_sources(${PROJECT_NAME} PRIVATE
  ${IMGUI_SOURCE_DIR}/imgui.cpp
  ${IMGUI_SOURCE_DIR}/imgui_draw.cpp
  ${IMGUI_SOURCE_DIR}/imgui_widgets.cpp
  ${IMGUI_SOURCE_DIR}/imgui_demo.cpp
  ${IMGUI_SOURCE_DIR}/imgui_tables.cpp
  ${RLIMGUI_SOURCE_DIR}/rlImGui.cpp
  src/main.cpp
)

##Dear IMGUI
#add_subdirectory(vendor/imgui)
#target_link_libraries(${PROJECT_NAME} PRIVATE imgui)
