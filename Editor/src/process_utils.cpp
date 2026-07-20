#include "process_utils.h"

#include <cstdlib>
#include <iostream>
#include <string>

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
                     const std::filesystem::path *stdout_stderr_log_file,
                     const std::vector<std::filesystem::path> *runtime_library_dirs,
                     const std::vector<std::pair<std::string, std::string>> *environment_overrides)
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

    std::wstring original_path;
    bool changed_path = false;
    std::vector<std::pair<std::wstring, std::wstring>> original_environment_values;
    std::vector<std::wstring> missing_environment_values;
    if (runtime_library_dirs != nullptr && !runtime_library_dirs->empty())
    {
      DWORD path_size = GetEnvironmentVariableW(L"PATH", nullptr, 0);
      if (path_size > 0)
      {
        original_path.resize(path_size - 1);
        GetEnvironmentVariableW(L"PATH", original_path.data(), path_size);
      }

      std::wstring child_path;
      for (const auto &directory : *runtime_library_dirs)
      {
        if (directory.empty())
        {
          continue;
        }
        if (!child_path.empty())
        {
          child_path += L";";
        }
        child_path += std::filesystem::absolute(directory).wstring();
      }
      if (!child_path.empty())
      {
        if (!original_path.empty())
        {
          child_path += L";" + original_path;
        }
        changed_path = SetEnvironmentVariableW(L"PATH", child_path.c_str()) != FALSE;
      }
    }

    if (environment_overrides != nullptr)
    {
      for (const auto &override_value : *environment_overrides)
      {
        const std::wstring name = std::filesystem::path(override_value.first).wstring();
        const std::wstring value = std::filesystem::path(override_value.second).wstring();
        DWORD value_size = GetEnvironmentVariableW(name.c_str(), nullptr, 0);
        if (value_size > 0)
        {
          std::wstring original_value(value_size - 1, L'\0');
          GetEnvironmentVariableW(name.c_str(), original_value.data(), value_size);
          original_environment_values.push_back({name, original_value});
        }
        else
        {
          missing_environment_values.push_back(name);
        }
        SetEnvironmentVariableW(name.c_str(), value.c_str());
      }
    }

    const BOOL created = CreateProcessW(exe_path.c_str(), command_line.data(), nullptr, nullptr, inherit_handles, 0, nullptr,
                                        working_dir_str.empty() ? nullptr : working_dir_str.c_str(), &startup_info, &process_info);
    for (const auto &original_value : original_environment_values)
    {
      SetEnvironmentVariableW(original_value.first.c_str(), original_value.second.c_str());
    }
    for (const auto &name : missing_environment_values)
    {
      SetEnvironmentVariableW(name.c_str(), nullptr);
    }
    if (changed_path)
    {
      SetEnvironmentVariableW(L"PATH", original_path.empty() ? nullptr : original_path.c_str());
    }
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
                     const std::filesystem::path *stdout_stderr_log_file,
                     const std::vector<std::filesystem::path> *runtime_library_dirs,
                     const std::vector<std::pair<std::string, std::string>> *environment_overrides)
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

      if (runtime_library_dirs != nullptr && !runtime_library_dirs->empty())
      {
        std::string library_path;
        for (const auto &directory : *runtime_library_dirs)
        {
          if (directory.empty())
          {
            continue;
          }
          if (!library_path.empty())
          {
            library_path += ":";
          }
          library_path += std::filesystem::absolute(directory).string();
        }
        const char *existing_library_path = std::getenv("LD_LIBRARY_PATH");
        if (existing_library_path != nullptr && existing_library_path[0] != '\0')
        {
          if (!library_path.empty())
          {
            library_path += ":";
          }
          library_path += existing_library_path;
        }
        if (!library_path.empty())
        {
          setenv("LD_LIBRARY_PATH", library_path.c_str(), 1);
        }
      }

      if (environment_overrides != nullptr)
      {
        for (const auto &override_value : *environment_overrides)
        {
          if (!override_value.first.empty())
          {
            setenv(override_value.first.c_str(), override_value.second.c_str(), 1);
          }
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
