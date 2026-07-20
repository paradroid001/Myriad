cmake_minimum_required(VERSION "3.18.4")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE TRUE)
project("MyriadEditor" LANGUAGES CXX VERSION 0.0.1 DESCRIPTION "Editor for Myriad Game Engine")
set(CMAKE_CXX_STANDARD 17)
add_executable(${PROJECT_NAME})
myriad_apply_mingw_libgcc(${PROJECT_NAME})

if(APPLE)
  set(MYRIAD_EDITOR_INSTALL_RPATH "@loader_path")
elseif(UNIX)
  set(MYRIAD_EDITOR_INSTALL_RPATH "$ORIGIN")
else()
  set(MYRIAD_EDITOR_INSTALL_RPATH "")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
  INSTALL_RPATH "${MYRIAD_EDITOR_INSTALL_RPATH}"
)

set(MYRIAD_EDITOR_INSTALL_COMPONENT "MyriadEditor")

# compile with all warnings, and warnings as errors. No rtti or exceptions, as they are not needed and add overhead.
# note: taking out -Werror as dependency rlImGui has an unused variable that generates a warning, and I don't want to have to patch their code.
target_compile_options(${PROJECT_NAME} PRIVATE -std=c++17;-Wall;-fno-rtti;-fno-exceptions)
#target_compile_options(${PROJECT_NAME} PRIVATE -fexceptions)

#link against myriad:
target_link_libraries(${PROJECT_NAME} MyriadEngine)
#link against raylib - I would really like to not have to do this.
target_link_libraries(${PROJECT_NAME} raylib)

if(WIN32)
  target_link_libraries(${PROJECT_NAME} ws2_32)
endif()

#Imgui Includes
set(IMGUI_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui)
set(RLIMGUI_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/rlImGui)
target_include_directories(${PROJECT_NAME} PRIVATE ${IMGUI_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${RLIMGUI_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vendor/cereal/include)
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
  src/build_bridge.cpp
  src/editor_app.cpp
  src/editor_settings.cpp
  src/json_utils.cpp
  src/main.cpp
  src/process_utils.cpp
  src/project_paths.cpp
)

##Dear IMGUI
#add_subdirectory(vendor/imgui)
#target_link_libraries(${PROJECT_NAME} PRIVATE imgui)

install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION "${MYRIAD_INSTALL_BIN_DIR}"
  COMPONENT ${MYRIAD_EDITOR_INSTALL_COMPONENT}
)

install(FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/themes.json"
  "${CMAKE_CURRENT_SOURCE_DIR}/layouts.json"
  DESTINATION "${MYRIAD_INSTALL_SHARE_DIR}/myriad-editor"
  COMPONENT ${MYRIAD_EDITOR_INSTALL_COMPONENT}
)

install(FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/default-config.json"
  DESTINATION "${MYRIAD_INSTALL_SHARE_DIR}/myriad-editor"
  RENAME ".myriad-editor.json"
  COMPONENT ${MYRIAD_EDITOR_INSTALL_COMPONENT}
)

if(MINGW)
  install(FILES "$<TARGET_FILE:MyriadEngine>"
    DESTINATION "${MYRIAD_INSTALL_BIN_DIR}"
    COMPONENT ${MYRIAD_EDITOR_INSTALL_COMPONENT}
  )
  install(FILES "$<TARGET_FILE:raylib>"
    DESTINATION "${MYRIAD_INSTALL_BIN_DIR}"
    COMPONENT ${MYRIAD_EDITOR_INSTALL_COMPONENT}
  )
endif()

# Copy resolved runtime shared-library dependencies next to the installed editor executable.
if(NOT MINGW)
  install(CODE "
    file(GET_RUNTIME_DEPENDENCIES
      EXECUTABLES \"$<TARGET_FILE:MyriadEditor>\"
      RESOLVED_DEPENDENCIES_VAR _myriad_editor_resolved_deps
      UNRESOLVED_DEPENDENCIES_VAR _myriad_editor_unresolved_deps
    )

    if(_myriad_editor_unresolved_deps)
      message(WARNING \"Unresolved runtime deps for MyriadEditor: \${_myriad_editor_unresolved_deps}\")
    endif()

    if(_myriad_editor_resolved_deps)
      file(INSTALL
        DESTINATION \"${CMAKE_INSTALL_PREFIX}/${MYRIAD_INSTALL_BIN_DIR}\"
        TYPE SHARED_LIBRARY
        FOLLOW_SYMLINK_CHAIN
        FILES \${_myriad_editor_resolved_deps}
      )
    endif()
  " COMPONENT ${MYRIAD_EDITOR_INSTALL_COMPONENT})
endif()
