#pragma once

#include "editor_model.h"
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
  /** @brief Refreshes the bridge-backed projects directory browser. */
  void RefreshProjectBrowser(const std::string &relative_path);
  /** @brief Refreshes the bridge-backed export directory browser. */
  void RefreshExportDirectoryBrowser(const std::string &relative_path);
  /** @brief Refreshes the project-local source directory browser. */
  void RefreshSourceDirectoryBrowser(const std::string &relative_path);
  /** @brief Refreshes the project-local resources directory browser. */
  void RefreshResourcesDirectoryBrowser(const std::string &relative_path);
  /** @brief Refreshes bridge-backed compiler toolkit and build type options. */
  void RefreshBuildOptions();
  /** @brief Opens a project selected from the bridge projects mount. */
  void OpenBridgeProject(const std::string &relative_path);
  /** @brief Creates and opens a project through the bridge projects mount. */
  void CreateBridgeProject(const std::string &parent_path, const std::string &project_name);
  /** @brief Creates a directory through the bridge projects mount. */
  bool CreateBridgeDirectory(const std::string &parent_path, const std::string &directory_name, std::string &created_relative_path);
  /** @brief Applies a resolved project root and reloads project-scoped settings/resources. */
  void ApplyProjectDirectory(const std::filesystem::path &project_root, const std::string &project_mount_path, bool reload_project_settings);
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
  Editor::EditorModel model_;
#if MYRIAD_EDITOR_ENABLE_BRIDGE_THREADS
  std::thread bridge_build_thread_;
  std::mutex bridge_build_mutex_;
#endif
};
