#pragma once

#include "editor_types.h"

#include <filesystem>
#include <string>
#include <vector>

namespace Editor
{
  /**
   * @brief Returns the default command template used for local CMake builds.
   */
  std::string DefaultBuildCommandTemplate();

  /**
   * @brief Parses a comma/space/semicolon-delimited bridge host list.
   * @param host_list User-provided host list string.
   * @return De-duplicated normalized host names.
   */
  std::vector<std::string> ParseHostList(const std::string &host_list);

  /**
   * @brief Builds an ordered list of bridge hosts to attempt.
   * @param settings Editor settings with preferred hosts.
   * @return Candidate hosts in connection attempt order.
   */
  std::vector<std::string> BuildBridgeHostCandidates(const EditorSettings &settings);

  /**
   * @brief Indicates whether the editor should route builds through the bridge.
   * @param settings Current editor settings.
   * @return true when socket build mode is enabled.
   */
  bool ShouldUseSocketBuilds(const EditorSettings &settings);

  /**
   * @brief Loads editor settings from environment variables and persisted config files.
   * @param root Project root directory used to resolve project-local or installed config paths.
   * @return Loaded settings with defaults applied.
   */
  EditorSettings LoadEditorSettings(const std::filesystem::path &root);

  /**
   * @brief Resolves the effective settings file path used for reading.
   * @param root Project root directory used during path resolution.
   * @return Settings path candidate; may not exist.
   */
  std::filesystem::path GetEditorSettingsReadPath(const std::filesystem::path &root);

  /**
   * @brief Resolves the effective settings file path used for writing.
   * @param root Project root directory used during path resolution.
   * @return Settings write destination path.
   */
  std::filesystem::path GetEditorSettingsWritePath(const std::filesystem::path &root);

  /**
   * @brief Describes the settings/config paths considered during startup.
   * @param root Project root directory used during path resolution.
   * @return Human-readable lines suitable for shell startup logging.
   */
  std::vector<std::string> GetEditorSettingsDiscoveryLog(const std::filesystem::path &root);

  /**
   * @brief Saves editor settings to the project root (source tree) or user config directory (installed app).
   * @param root Project root directory used to resolve the write destination.
   * @param settings Settings object to persist.
   * @return true when the file was written successfully.
   */
  bool SaveEditorSettings(const std::filesystem::path &root, const EditorSettings &settings);

  /**
   * @brief Resolves the effective build command template.
   * @param settings Current editor settings.
   * @return User template when provided, otherwise default template.
   */
  std::string ResolveBuildCommandTemplate(const EditorSettings &settings);

  /**
   * @brief Converts an absolute build directory to a project-relative bridge path.
   * @param build_dir Absolute build directory.
   * @param project_root Project root used as the relative base.
   * @return Relative path text, or empty string when conversion is unsafe/invalid.
   */
  std::string BuildDirForBridge(const std::filesystem::path &build_dir, const std::filesystem::path &project_root);

  /**
   * @brief Creates a JSON build request payload for the bridge.
   * @param preset Selected compiler preset.
   * @param build_dir_relative Build directory relative to project root.
   * @return Serialized JSON request line including trailing newline.
   */
  std::string CreateBuildBridgeRequest(const CompilerPreset &preset, const std::string &build_dir_relative);

  /**
   * @brief Creates a JSON status probe request for the bridge.
   * @return Serialized JSON request line including trailing newline.
   */
  std::string CreateBuildBridgeStatusRequest();
} // namespace Editor
