#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace Editor
{
  /**
   * @brief Build toolchain preset loaded from CMakeKits.json.
   */
  struct CompilerPreset
  {
    /** @brief Display name used in editor selection UI. */
    std::string name;
    /** @brief Optional path to the toolchain file for this preset. */
    std::filesystem::path toolchain_file;
  };

  /**
   * @brief Named ImGui theme preset represented by hex color values.
   */
  struct ThemePreset
  {
    /** @brief User-facing preset name. */
    std::string name;
    /** @brief Base ImGui style name: dark, light, or classic. */
    std::string base = "dark";
    /** @brief Main window background color. */
    std::string window_bg_hex;
    /** @brief Header color. */
    std::string header_hex;
    /** @brief Header hover color. */
    std::string header_hovered_hex;
    /** @brief Button color. */
    std::string button_hex;
    /** @brief Button hover color. */
    std::string button_hovered_hex;
    /** @brief Frame background color. */
    std::string frame_bg_hex;
    /** @brief Inactive title bar color. */
    std::string title_bg_hex;
    /** @brief Active title bar color. */
    std::string title_bg_active_hex;

    template <class Archive>
    void serialize(Archive &archive)
    {
      archive(name, base, window_bg_hex, header_hex, header_hovered_hex, button_hex, button_hovered_hex, frame_bg_hex, title_bg_hex, title_bg_active_hex);
    }
  };

  /**
   * @brief Dock layout percentage preset used for editor windows.
   */
  struct LayoutPreset
  {
    /** @brief User-facing layout name. */
    std::string name;
    /** @brief Layout mode identifier used by layout code. */
    std::string mode = "balanced";
    /** @brief Left panel width percentage. */
    int left_percent = 25;
    /** @brief Right panel width percentage. */
    int right_percent = 28;
    /** @brief Bottom panel height percentage. */
    int bottom_percent = 30;

    template <class Archive>
    void serialize(Archive &archive)
    {
      archive(name, mode, left_percent, right_percent, bottom_percent);
    }
  };

  /**
   * @brief Named set of build parameters stored per project.
   */
  struct BuildProfile
  {
    /** @brief User-facing profile name. */
    std::string name = "Default";
    /** @brief Build mode selector such as local or socket. */
    std::string build_mode;
    /** @brief Preferred build bridge host. */
    std::string build_socket_host;
    /** @brief Fallback build bridge host list in priority order. */
    std::vector<std::string> build_socket_hosts;
    /** @brief Build bridge TCP port. */
    int build_socket_port = 55333;
    /** @brief Connectivity probe cadence in seconds for bridge status checks. */
    int build_bridge_probe_interval_seconds = 30;
    /** @brief Local build command template when bridge mode is disabled. */
    std::string build_command_template;
    /** @brief User-facing project name shown by the editor. */
    std::string project_name = "TestECS";
    /** @brief Selected compiler toolkit name. */
    std::string compiler_toolkit;
    /** @brief Selected CMake build type. */
    std::string build_type = "Debug";
    /** @brief CMake target and executable name to build, run, and export. */
    std::string target_executable_name = "TestECS";
    /** @brief Source-side CMake source directory used for game builds. */
    std::string source_directory;
    /** @brief Project-relative directory containing runtime resources/assets. */
    std::string resources_directory;
    /** @brief Build directory stored relative to the project root when possible. */
    std::string build_directory;
    /** @brief Header/include directories used by local build command placeholders. */
    std::string header_search_dirs;
    /** @brief Library directories used by local build command placeholders. */
    std::string library_search_dirs;
    /** @brief Source-side projects mount directory where successful bridge builds are exported. */
    std::string export_directory;
    /** @brief Last selected executable path string for this profile. */
    std::string last_executable_path;

    template <class Archive>
    void serialize(Archive &archive)
    {
      archive(name, build_mode, build_socket_host, build_socket_hosts, build_socket_port, build_bridge_probe_interval_seconds,
              build_command_template, project_name, compiler_toolkit, build_type, target_executable_name, source_directory,
              resources_directory, build_directory, header_search_dirs, library_search_dirs, export_directory, last_executable_path);
    }
  };

  /**
   * @brief Persisted editor configuration stored in grouped editor.json files.
   */
  struct EditorSettings
  {
    /** @brief Build mode selector such as local or socket. */
    std::string build_mode;
    /** @brief Preferred build bridge host. */
    std::string build_socket_host;
    /** @brief Fallback build bridge host list in priority order. */
    std::vector<std::string> build_socket_hosts;
    /** @brief Build bridge TCP port. */
    int build_socket_port = 55333;
    /** @brief Connectivity probe cadence in seconds for bridge status checks. */
    int build_bridge_probe_interval_seconds = 30;
    /** @brief Local build command template when bridge mode is disabled. */
    std::string build_command_template;
    /** @brief User-facing project name shown and persisted by the editor. */
    std::string project_name = "TestECS";
    /** @brief Selected compiler toolkit name for the current project. */
    std::string compiler_toolkit;
    /** @brief Selected CMake build type for the current project. */
    std::string build_type = "Debug";
    /** @brief CMake target and executable name to build, run, and export. */
    std::string target_executable_name = "TestECS";
    /** @brief Source-side CMake source directory used for game builds. */
    std::string source_directory;
    /** @brief Project-relative directory containing runtime resources/assets. */
    std::string resources_directory;
    /** @brief Build directory stored relative to the project root when possible. */
    std::string build_directory;
    /** @brief Project root directory selected for editor workflows. */
    std::string project_root_path;
    /** @brief Project directory relative to the build bridge projects mount. */
    std::string project_mount_path;
    /** @brief Header/include directories used by local build command placeholders. */
    std::string header_search_dirs;
    /** @brief Library directories used by local build command placeholders. */
    std::string library_search_dirs;
    /** @brief Source-side projects mount directory where successful bridge builds are exported. */
    std::string export_directory;
    /** @brief Active build profile name. */
    std::string selected_build_profile = "Default";
    /** @brief Named build profile definitions stored with the project. */
    std::vector<BuildProfile> build_profiles;
    /** @brief Last selected theme preset name. */
    std::string theme_preset = "Dark";
    /** @brief UI font scale in percent. */
    int ui_font_scale_percent = 100;
    /** @brief UI corner rounding radius. */
    int ui_rounding = 6;
    /** @brief UI spacing density in percent. */
    int ui_spacing_percent = 100;
    /** @brief Accent color in hex format. */
    std::string ui_accent_hex = "#4D8ACF";

    /** @brief Last selected compiler preset name. */
    std::string last_compiler_preset;
    /** @brief Last selected build directory path string. */
    std::string last_build_dir;
    /** @brief Last selected executable path string. */
    std::string last_executable_path;
    /** @brief Last selected dock layout preset index. */
    int layout_preset_index = 0;

    /** @brief Saved main editor window width in pixels. */
    int editor_window_width = 0;
    /** @brief Saved main editor window height in pixels. */
    int editor_window_height = 0;

    /** @brief Persisted visibility for Build Workflow panel. */
    bool panel_build_workflow_open = true;
    /** @brief Persisted visibility for Scene panel. */
    bool panel_scene_open = true;
    /** @brief Persisted visibility for Preview panel. */
    bool panel_preview_open = true;
    /** @brief Persisted visibility for Console panel. */
    bool panel_console_open = true;
    /** @brief Persisted visibility for Game Log panel. */
    bool panel_game_log_open = true;
    /** @brief Stored Build Workflow panel width in pixels. */
    int panel_build_workflow_width = 560;
    /** @brief Stored Build Workflow panel height in pixels. */
    int panel_build_workflow_height = 360;
    /** @brief Stored Preview panel width in pixels. */
    int panel_preview_width = 820;
    /** @brief Stored Preview panel height in pixels. */
    int panel_preview_height = 620;
    /** @brief Stored Console panel width in pixels. */
    int panel_console_width = 760;
    /** @brief Stored Console panel height in pixels. */
    int panel_console_height = 260;
    /** @brief Stored Scene panel width in pixels. */
    int panel_scene_width = 900;
    /** @brief Stored Scene panel height in pixels. */
    int panel_scene_height = 640;

    template <class Archive>
    void serialize(Archive &archive)
    {
      archive(build_mode, build_socket_host, build_socket_hosts, build_socket_port, build_bridge_probe_interval_seconds,
              build_command_template, project_name, compiler_toolkit, build_type, target_executable_name, source_directory,
              resources_directory, build_directory, project_root_path, project_mount_path, header_search_dirs, library_search_dirs, export_directory,
              selected_build_profile, build_profiles, theme_preset,
              ui_font_scale_percent, ui_rounding, ui_spacing_percent, ui_accent_hex, last_compiler_preset,
              last_build_dir, last_executable_path, layout_preset_index, editor_window_width, editor_window_height,
              panel_build_workflow_open, panel_scene_open, panel_preview_open, panel_console_open, panel_game_log_open,
              panel_build_workflow_width, panel_build_workflow_height, panel_preview_width, panel_preview_height,
              panel_console_width, panel_console_height, panel_scene_width, panel_scene_height);
    }
  };
} // namespace Editor
