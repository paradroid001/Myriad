#pragma once

#include "editor_settings.h"
#include "editor_types.h"

#include "myriad.h"
#include "raylib.h"

#include <filesystem>
#include <atomic>
#include <memory>
#include <string>
#include <vector>

#if defined(__MINGW32__)
#define MYRIAD_EDITOR_ENABLE_BRIDGE_THREADS 0
#else
#define MYRIAD_EDITOR_ENABLE_BRIDGE_THREADS 1
#include <mutex>
#include <thread>
#endif

#include "imgui.h"

/**
 * @brief Main Myriad editor application with docking UI, build workflow, and live preview.
 */
class MyriadEditor : public Myriad::MyrGameApplication
{
public:
  MyriadEditor() = default;
  /** @brief Releases editor resources and hosted preview state. */
  ~MyriadEditor() override;

  /**
   * @brief Performs one-time editor initialization before Start.
   * @param config Engine configuration used to bootstrap editor runtime.
   */
  void Init(Myriad::GameEngineConfig &config) override;
  /** @brief Starts editor subsystems after initialization. */
  void Start() override;
  /** @brief Renders one editor frame and processes UI interactions. */
  void Render() override;
  /** @brief Runs shutdown cleanup before engine teardown. */
  void PreShutdown() override;

private:
  /** @brief Appends a line to the in-editor console log. */
  void AppendConsoleLine(const std::string &line);
  /** @brief Starts the embedded preview runtime. */
  void StartPreviewGame();
  /** @brief Stops the embedded preview runtime. */
  void StopPreviewGame();
  /** @brief Applies a named theme preset by index. */
  void ApplyThemePresetByIndex(int preset_index);
  /** @brief Applies ad-hoc theme customizations from current UI settings. */
  void ApplyThemeCustomizations();
  /** @brief Persists theme-related settings to the editor settings file. */
  void PersistThemePreference();
  /** @brief Refreshes resolved build/output paths from current selections. */
  void RefreshPaths(bool force_defaults = false);
  /** @brief Updates build bridge connectivity and rebuild-needed status. */
  void RefreshBuildBridgeStatus(bool force = false);
  /** @brief Starts an asynchronous build request through the socket bridge. */
  void StartBridgeBuildOverSocket(const Editor::CompilerPreset &preset, const std::string &build_dir_relative);
  /** @brief Drains pending bridge build output and applies final build results. */
  void PumpBridgeBuildUpdates();
  /** @brief Parses streamed bridge output to update build progress state. */
  void UpdateBridgeBuildProgressFromLine(const std::string &line);
  /** @brief Joins the bridge worker thread when it has completed. */
  void JoinBridgeBuildThread();
  /** @brief Restarts hosted preview after a successful build when needed. */
  void RestartPreviewForLatestBuild();
  /** @brief Reloads hosted preview game code from the latest build artifact. */
  bool ReloadHostedPreviewLibrary();
  /** @brief Unloads hosted preview game library resources. */
  void UnloadHostedPreviewLibrary();
  /** @brief Executes a TestECS build using either local or bridge workflow. */
  void BuildTestECS();
  /** @brief Launches the built TestECS executable. */
  void RunTestECS();
  /** @brief Stops the launched game process, if running. */
  void StopGame();
  /** @brief Polls and appends runtime log output from the game process. */
  void PumpGameLogOutput();
  /** @brief Starts redirecting hosted preview stdout/stderr to the game log panel. */
  void BeginPreviewLogCapture();
  /** @brief Restores stdout/stderr after hosted preview log capture. */
  void EndPreviewLogCapture();

private:
  std::filesystem::path project_root_;
  std::filesystem::path build_dir_;
  std::filesystem::path game_executable_;
  std::vector<Editor::CompilerPreset> compiler_presets_;
  std::vector<Editor::ThemePreset> theme_presets_;
  std::vector<Editor::LayoutPreset> layout_presets_;
  Editor::EditorSettings editor_settings_;
  std::string build_command_template_;
  bool build_bridge_connected_ = false;
  std::string build_bridge_status_text_;
  int build_bridge_consecutive_failures_ = 0;
  bool build_bridge_warning_active_ = false;
  double build_bridge_last_probe_time_ = 0.0;
  Myriad::MyrGameApplication *preview_game_ = nullptr;
  using HostedDestroyFn = void (*)(Myriad::MyrGameApplication *);
  HostedDestroyFn hosted_destroy_fn_ = nullptr;
  void *hosted_library_handle_ = nullptr;
  bool hosted_library_reload_required_ = true;
  std::filesystem::path hosted_library_path_;
  std::filesystem::path hosted_library_loaded_copy_path_;
  RenderTexture2D preview_texture_ = {};
  bool preview_texture_ready_ = false;
  bool preview_texture_cleanup_requested_ = false;
  bool preview_stop_requested_ = false;
  double preview_last_tick_time_ = 0.0;
  std::string build_dir_input_;
  std::string executable_input_;
  std::string status_;
  std::vector<std::string> console_lines_;
  bool build_succeeded_ = false;
  bool show_build_workflow_window_ = true;
  bool show_scene_window_ = true;
  bool show_preview_window_ = true;
  bool show_console_window_ = true;
  bool show_game_log_window_ = true;
  bool show_editor_preferences_window_ = true;
  bool console_scroll_to_bottom_ = false;
  bool game_log_scroll_to_bottom_ = false;
#if MYRIAD_EDITOR_ENABLE_BRIDGE_THREADS
  std::thread bridge_build_thread_;
  std::mutex bridge_build_mutex_;
#endif
  std::vector<std::string> bridge_build_pending_lines_;
  std::atomic<bool> bridge_build_in_progress_ = false;
  bool bridge_build_result_ready_ = false;
  bool run_after_build_request_ = false;
  bool bridge_build_request_success_ = false;
  bool bridge_build_success_ = false;
  bool bridge_build_rebuild_triggered_ = false;
  std::string bridge_build_response_;
  std::string bridge_build_error_;
  std::string bridge_build_status_;
  std::string bridge_build_executable_;
  std::string bridge_build_host_;
  int bridge_build_progress_percent_ = -1;
  bool bridge_rebuild_needed_ = false;
  int bridge_changed_file_count_ = 0;
  std::vector<std::string> bridge_changed_files_preview_;
  std::vector<std::string> game_log_lines_;
  int selected_theme_preset_index_ = 0;
  float ui_font_scale_ = 1.0f;
  float ui_rounding_ = 6.0f;
  float ui_spacing_density_ = 1.0f;
  ImVec4 ui_accent_color_ = ImVec4(0.30f, 0.54f, 0.81f, 1.0f);
  ImGuiStyle base_theme_style_ = {};
  bool has_base_theme_style_ = false;
  int selected_layout_preset_index_ = 0;
  bool dock_layout_apply_requested_ = true;
  int selected_preset_index_ = 0;
  int last_selected_preset_index_ = -1;
  std::intptr_t game_pid_ = -1;
#ifndef _WIN32
  int preview_log_pipe_read_fd_ = -1;
  int preview_log_pipe_write_fd_ = -1;
  int preview_log_saved_stdout_fd_ = -1;
  int preview_log_saved_stderr_fd_ = -1;
  std::string preview_log_partial_line_;
#endif
};
