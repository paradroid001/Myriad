#include "process_utils.h"

#include <iostream>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <process.h>
#include <windows.h>
#else
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace Editor
{
#ifdef _WIN32
  bool LaunchProcess(const std::filesystem::path &executable,
                     const std::filesystem::path &working_dir,
                     std::intptr_t &process_id,
                     const std::filesystem::path *stdout_stderr_log_file)
  {
    const std::filesystem::path absolute_executable = std::filesystem::absolute(executable);
    const std::wstring exe_path = absolute_executable.wstring();
    std::wstring command_line = L"\"" + exe_path + L"\"";
    const std::wstring working_dir_str = std::filesystem::absolute(working_dir).wstring();

    STARTUPINFOW startup_info{};
    startup_info.cb = sizeof(startup_info);
    PROCESS_INFORMATION process_info{};
    HANDLE log_handle = nullptr;
    BOOL inherit_handles = FALSE;

    if (stdout_stderr_log_file != nullptr && !stdout_stderr_log_file->empty())
    {
      std::error_code create_error;
      std::filesystem::create_directories(stdout_stderr_log_file->parent_path(), create_error);

      SECURITY_ATTRIBUTES security_attributes{};
      security_attributes.nLength = sizeof(security_attributes);
      security_attributes.bInheritHandle = TRUE;
      log_handle = CreateFileW(stdout_stderr_log_file->wstring().c_str(),
                               FILE_APPEND_DATA,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               &security_attributes,
                               OPEN_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL,
                               nullptr);
      if (log_handle == INVALID_HANDLE_VALUE)
      {
        log_handle = nullptr;
      }
      else
      {
        SetFilePointer(log_handle, 0, nullptr, FILE_END);
        startup_info.dwFlags |= STARTF_USESTDHANDLES;
        startup_info.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        startup_info.hStdOutput = log_handle;
        startup_info.hStdError = log_handle;
        inherit_handles = TRUE;
      }
    }

    const BOOL created = CreateProcessW(exe_path.c_str(), command_line.data(), nullptr, nullptr, inherit_handles, 0, nullptr,
                                        working_dir_str.empty() ? nullptr : working_dir_str.c_str(), &startup_info, &process_info);
    if (log_handle != nullptr)
    {
      CloseHandle(log_handle);
    }

    if (!created)
    {
      return false;
    }

    process_id = static_cast<std::intptr_t>(process_info.dwProcessId);
    CloseHandle(process_info.hThread);
    CloseHandle(process_info.hProcess);
    return true;
  }

  bool StopProcess(std::intptr_t process_id)
  {
    if (process_id <= 0)
    {
      return false;
    }

    const HANDLE process = OpenProcess(PROCESS_TERMINATE, FALSE, static_cast<DWORD>(process_id));
    if (process == nullptr)
    {
      return false;
    }

    const BOOL terminated = TerminateProcess(process, 1);
    CloseHandle(process);
    return terminated != FALSE;
  }
#else
  bool LaunchProcess(const std::filesystem::path &executable,
                     const std::filesystem::path &working_dir,
                     std::intptr_t &process_id,
                     const std::filesystem::path *stdout_stderr_log_file)
  {
    const pid_t pid = fork();
    if (pid == 0)
    {
      chdir(working_dir.c_str());

      if (stdout_stderr_log_file != nullptr && !stdout_stderr_log_file->empty())
      {
        std::error_code create_error;
        std::filesystem::create_directories(stdout_stderr_log_file->parent_path(), create_error);
        const int log_fd = open(stdout_stderr_log_file->c_str(), O_CREAT | O_WRONLY | O_APPEND, 0644);
        if (log_fd >= 0)
        {
          dup2(log_fd, STDOUT_FILENO);
          dup2(log_fd, STDERR_FILENO);
          close(log_fd);
        }
      }

      execl(executable.c_str(), executable.filename().c_str(), nullptr);
      std::cerr << "Failed to launch TestECS from " << executable << std::endl;
      _exit(127);
    }

    if (pid > 0)
    {
      process_id = static_cast<std::intptr_t>(pid);
      return true;
    }

    return false;
  }

  bool StopProcess(std::intptr_t process_id)
  {
    if (process_id <= 0)
    {
      return false;
    }

    const pid_t pid = static_cast<pid_t>(process_id);
    kill(pid, SIGTERM);
    waitpid(pid, nullptr, 0);
    return true;
  }
#endif
} // namespace Editor
