#pragma once

#include "editor_types.h"

#include <filesystem>
#include <string>
#include <vector>

namespace Editor
{
  /**
   * @brief Expands placeholders in a build command template.
   * @param command Template containing placeholders like {projectRoot} and {buildDir}.
   * @param project_root Project root directory.
   * @param build_dir Selected build directory.
   * @param preset Active compiler preset.
   * @param toolchain_path Optional resolved toolchain file path.
   * @return Expanded command string ready for execution.
   */
  std::string ExpandBuildCommandTemplate(std::string command,
                                         const std::filesystem::path &project_root,
                                         const std::filesystem::path &build_dir,
                                         const CompilerPreset &preset,
                                         const std::filesystem::path &toolchain_path);

  /**
   * @brief Locates the repository root for the running editor process.
   * @return Absolute path to project root when found.
   */
  std::filesystem::path FindProjectRoot();

  /**
   * @brief Loads compiler presets from CMakeKits.json.
   * @param root Project root directory.
   * @return Parsed preset list, possibly empty.
   */
  std::vector<CompilerPreset> LoadCompilerPresets(const std::filesystem::path &root);

  /**
   * @brief Resolves a potentially relative path against a root directory.
   * @param root Base directory.
   * @param path Path to resolve.
   * @return Absolute normalized path when possible.
   */
  std::filesystem::path ResolvePath(const std::filesystem::path &root, const std::filesystem::path &path);

  /**
   * @brief Finds a build directory matching a preset name under build/.
   * @param root Project root directory.
   * @param preset_name Preset name selected in the editor.
   * @return Matching build directory or empty path when not found.
   */
  std::filesystem::path FindMatchingBuildDirectory(const std::filesystem::path &root, const std::string &preset_name);

  /**
   * @brief Resolves an executable path relative to build or project roots.
   * @param build_dir Selected build directory.
   * @param project_root Project root directory.
   * @param path User-provided executable path.
   * @return Resolved executable candidate path.
   */
  std::filesystem::path ResolveExecutablePath(const std::filesystem::path &build_dir,
                                              const std::filesystem::path &project_root,
                                              const std::filesystem::path &path);

  /**
   * @brief Attempts to locate the TestECS runtime executable.
   * @param build_dir Selected build directory.
   * @param override Explicit executable override path.
   * @param project_root Project root directory.
   * @return Best executable path candidate or empty path.
   */
  std::filesystem::path FindGameExecutable(const std::filesystem::path &build_dir,
                                           const std::filesystem::path &override,
                                           const std::filesystem::path &project_root);
} // namespace Editor
