#pragma once

#include "editor_types.h"

#include "imgui.h"
#include "myriad.h"
#include "raylib.h"

#include <atomic>
#include <filesystem>
#include <string>
#include <vector>

namespace Editor
{
  using HostedDestroyFn = void (*)(Myriad::MyrGameApplication *);

  /** @brief Plain editor model state shared by the controller and ImGui view. */
  struct EditorModel
  {
    std::filesystem::path project_root;
    std::filesystem::path build_dir;
    std::filesystem::path game_executable;
    std::vector<CompilerPreset> compiler_presets;
    std::vector<ThemePreset> theme_presets;
    std::vector<LayoutPreset> layout_presets;
    EditorSettings settings;
    std::string build_command_template;

    bool build_bridge_connected = false;
    std::string build_bridge_status_text;
    int build_bridge_consecutive_failures = 0;
    bool build_bridge_warning_active = false;
    double build_bridge_last_probe_time = 0.0;
    std::vector<std::string> bridge_build_pending_lines;
    std::atomic<bool> bridge_build_in_progress = false;
    bool bridge_build_result_ready = false;
    bool run_after_build_request = false;
    bool bridge_build_request_success = false;
    bool bridge_build_success = false;
    bool bridge_build_rebuild_triggered = false;
    std::string bridge_build_response;
    std::string bridge_build_error;
    std::string bridge_build_status;
    std::string bridge_build_executable;
    std::string bridge_build_host;
    int bridge_build_progress_percent = -1;
    bool bridge_rebuild_needed = false;
    int bridge_changed_file_count = 0;
    std::vector<std::string> bridge_changed_files_preview;

    std::string project_browser_mount_path;
    std::string project_mount_source_path;
    std::string dist_mount_source_path;
    std::string default_header_search_dirs;
    std::string default_library_search_dirs;
    std::string project_browser_relative_path;
    std::string project_browser_status;
    std::string new_project_name;
    std::string export_browser_relative_path;
    std::string export_browser_status;
    std::string new_export_directory_name;
    std::string source_browser_relative_path;
    std::string source_browser_status;
    std::string resources_browser_relative_path;
    std::string resources_browser_status;
    std::vector<std::string> build_type_options;
    std::vector<std::string> project_browser_directories;
    std::vector<std::string> export_browser_directories;
    std::vector<std::string> source_browser_directories;
    std::vector<std::string> resources_browser_directories;
    bool show_open_project_dialog = false;
    bool show_new_project_dialog = false;
    bool show_export_directory_dialog = false;
    bool show_source_directory_dialog = false;
    bool show_resources_directory_dialog = false;

    Myriad::MyrGameApplication *preview_game = nullptr;
    HostedDestroyFn hosted_destroy_fn = nullptr;
    void *hosted_library_handle = nullptr;
    bool hosted_library_reload_required = true;
    std::filesystem::path hosted_library_path;
    std::filesystem::path hosted_library_loaded_copy_path;
    RenderTexture2D preview_texture = {};
    bool preview_texture_ready = false;
    bool preview_texture_cleanup_requested = false;
    bool preview_stop_requested = false;
    double preview_last_tick_time = 0.0;

    std::string build_dir_input;
    std::string executable_input;
    std::string new_build_profile_name;
    std::string status;
    std::vector<std::string> console_lines;
    std::vector<std::string> game_log_lines;
    std::filesystem::path runtime_log_path;
    std::uintmax_t runtime_log_offset = 0;
    bool build_succeeded = false;
    bool show_project_window = true;
    bool show_scene_window = true;
    bool show_preview_window = true;
    bool show_console_window = true;
    bool show_game_log_window = true;
    bool project_settings_dirty = false;
    bool console_scroll_to_bottom = false;
    bool game_log_scroll_to_bottom = false;

    int selected_theme_preset_index = 0;
    float ui_font_scale = 1.0f;
    float ui_rounding = 6.0f;
    float ui_spacing_density = 1.0f;
    ImVec4 ui_accent_color = ImVec4(0.30f, 0.54f, 0.81f, 1.0f);
    ImGuiStyle base_theme_style = {};
    bool has_base_theme_style = false;
    int selected_layout_preset_index = 0;
    bool dock_layout_apply_requested = true;
    int selected_preset_index = 0;
    int last_selected_preset_index = -1;
    int selected_build_profile_index = 0;
    std::intptr_t game_pid = -1;

#ifndef _WIN32
    int preview_log_pipe_read_fd = -1;
    int preview_log_pipe_write_fd = -1;
    int preview_log_saved_stdout_fd = -1;
    int preview_log_saved_stderr_fd = -1;
    std::string preview_log_partial_line;
#endif
  };
} // namespace Editor
