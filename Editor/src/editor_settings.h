#pragma once

#include "editor_types.h"

#include <filesystem>
#include <string>
#include <vector>

namespace Editor
{
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
   * @param project_mount_path Project directory relative to the bridge projects mount.
   * @return Serialized JSON request line including trailing newline.
   */
  std::string CreateBuildBridgeRequest(const CompilerPreset &preset,
                                       const std::string &build_dir_relative,
                                       const std::string &project_mount_path,
                                       const std::string &build_type,
                                       const std::string &target_executable_name,
                                       const std::string &source_directory,
                                       const std::string &resources_directory,
                                       const std::string &header_search_dirs,
                                       const std::string &library_search_dirs,
                                       const std::string &export_directory);

  /**
   * @brief Creates a JSON request to list bridge-supported compiler toolkits and build types.
   * @param project_mount_path Project directory relative to the bridge projects mount.
   * @return Serialized JSON request line including trailing newline.
   */
  std::string CreateBuildOptionsRequest(const std::string &project_mount_path, const std::string &compiler_toolkit, const std::string &build_type);

  /**
   * @brief Creates a JSON request to list project directories under the bridge projects mount.
   * @param relative_path Directory relative to the projects mount.
   * @return Serialized JSON request line including trailing newline.
   */
  std::string CreateProjectListRequest(const std::string &relative_path);

  /**
   * @brief Creates a JSON request to resolve/open a project under the bridge projects mount.
   * @param relative_path Directory relative to the projects mount.
   * @return Serialized JSON request line including trailing newline.
   */
  std::string CreateProjectOpenRequest(const std::string &relative_path);

  /**
   * @brief Creates a JSON request to create a new project under the bridge projects mount.
   * @param parent_path Parent directory relative to the projects mount.
   * @param project_name New project directory name.
   * @return Serialized JSON request line including trailing newline.
   */
  std::string CreateProjectCreateRequest(const std::string &parent_path, const std::string &project_name);

  /**
   * @brief Creates a JSON request to list directories under the bridge projects mount.
   * @param relative_path Directory relative to the projects mount.
   * @return Serialized JSON request line including trailing newline.
   */
  std::string CreateDirectoryListRequest(const std::string &relative_path);

  /**
   * @brief Creates a JSON request to create a directory under the bridge projects mount.
   * @param parent_path Parent directory relative to the projects mount.
   * @param directory_name New directory name.
   * @return Serialized JSON request line including trailing newline.
   */
  std::string CreateDirectoryCreateRequest(const std::string &parent_path, const std::string &directory_name);

  /**
   * @brief Creates a JSON request to persist project settings through the build bridge.
   * @param settings Current project settings to write.
   * @return Serialized JSON request line including trailing newline.
   */
  std::string CreateProjectSettingsSaveRequest(const EditorSettings &settings);

  /**
   * @brief Creates a JSON status probe request for the bridge.
   * @param project_mount_path Project directory relative to the bridge projects mount.
   * @return Serialized JSON request line including trailing newline.
   */
  std::string CreateBuildBridgeStatusRequest(const std::string &project_mount_path, const std::string &target_executable_name, const std::string &source_directory);
} // namespace Editor
