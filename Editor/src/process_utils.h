#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace Editor
{
  /**
   * @brief Launches an external process for game execution.
   * @param executable Path to executable binary.
   * @param working_dir Process working directory.
   * @param process_id Receives spawned process identifier on success.
   * @param stdout_stderr_log_file Optional log file to capture stdout/stderr.
   * @param runtime_library_dirs Optional directories added to the child runtime library search path.
   * @param environment_overrides Optional environment variables set for the child process.
   * @return true when the process starts successfully.
   */
  bool LaunchProcess(const std::filesystem::path &executable,
                     const std::filesystem::path &working_dir,
                     std::intptr_t &process_id,
                     const std::filesystem::path *stdout_stderr_log_file = nullptr,
                     const std::vector<std::filesystem::path> *runtime_library_dirs = nullptr,
                     const std::vector<std::pair<std::string, std::string>> *environment_overrides = nullptr);

  /**
   * @brief Requests termination of a running process.
   * @param process_id Process identifier previously returned by LaunchProcess.
   * @return true when stop request succeeds.
   */
  bool StopProcess(std::intptr_t process_id);
} // namespace Editor
