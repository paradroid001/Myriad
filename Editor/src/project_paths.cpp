#include "project_paths.h"

#include "json_utils.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#else
#include <limits.h>
#include <unistd.h>
#endif

namespace Editor
{
  namespace
  {
    std::vector<std::filesystem::path> ParsePathList(const std::string &path_list)
    {
      std::vector<std::filesystem::path> paths;
      std::string current;
      for (const char ch : path_list)
      {
        if (ch == ';' || ch == '\n' || ch == '\r')
        {
          current = Trim(current);
          if (!current.empty())
          {
            paths.emplace_back(current);
          }
          current.clear();
          continue;
        }

        current.push_back(ch);
      }

      current = Trim(current);
      if (!current.empty())
      {
        paths.emplace_back(current);
      }

      return paths;
    }

    std::string BuildPathFlags(const std::string &path_list, const std::string &flag_name)
    {
      std::ostringstream flags;
      bool first = true;
      for (const auto &path : ParsePathList(path_list))
      {
        if (!first)
        {
          flags << ' ';
        }
        flags << flag_name << "\"" << path.string() << "\"";
        first = false;
      }
      return flags.str();
    }

    std::filesystem::path ResolveExecutablePathForProcess()
    {
#ifdef _WIN32
      char module_path[MAX_PATH] = {};
      const DWORD copied = GetModuleFileNameA(nullptr, module_path, MAX_PATH);
      if (copied == 0 || copied >= MAX_PATH)
      {
        return {};
      }
      return std::filesystem::path(module_path);
#elif defined(__APPLE__)
      uint32_t size = 0;
      _NSGetExecutablePath(nullptr, &size);
      if (size == 0)
      {
        return {};
      }

      std::string buffer(size, '\0');
      if (_NSGetExecutablePath(buffer.data(), &size) != 0)
      {
        return {};
      }

      return std::filesystem::weakly_canonical(std::filesystem::path(buffer.c_str()));
#else
      char buffer[PATH_MAX] = {};
      const ssize_t copied = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
      if (copied <= 0)
      {
        return {};
      }

      buffer[copied] = '\0';
      return std::filesystem::path(buffer);
#endif
    }
  } // namespace

  std::string ExpandBuildCommandTemplate(std::string command,
                                         const std::filesystem::path &project_root,
                                         const std::filesystem::path &build_dir,
                                         const CompilerPreset &preset,
                                         const std::filesystem::path &toolchain_path,
                                         const std::string &header_search_dirs,
                                         const std::string &library_search_dirs)
  {
    const std::string project_root_text = project_root.string();
    const std::string build_dir_text = build_dir.string();
    const std::string toolchain_text = toolchain_path.empty() ? std::string{} : toolchain_path.string();
    const std::string toolchain_arg = toolchain_text.empty() ? std::string{} : "-DCMAKE_TOOLCHAIN_FILE=\"" + toolchain_text + "\"";

    ReplaceAll(command, "{projectRoot}", project_root_text);
    ReplaceAll(command, "{buildDir}", build_dir_text);
    ReplaceAll(command, "{preset}", preset.name);
    ReplaceAll(command, "{target}", "TestECS");
    ReplaceAll(command, "{toolchainFile}", toolchain_text);
    ReplaceAll(command, "{toolchainArg}", toolchain_arg);
    ReplaceAll(command, "{headerDirs}", header_search_dirs);
    ReplaceAll(command, "{libraryDirs}", library_search_dirs);
    ReplaceAll(command, "{includeArgs}", BuildPathFlags(header_search_dirs, "-I"));
    ReplaceAll(command, "{libraryArgs}", BuildPathFlags(library_search_dirs, "-L"));
    return command;
  }

  std::filesystem::path FindProjectRoot()
  {
    const char *project_root_override = std::getenv("MYRIAD_PROJECT_ROOT");
    if (project_root_override != nullptr && *project_root_override != '\0')
    {
      const std::filesystem::path override_root = FindProjectRootFromPath(project_root_override);
      if (!override_root.empty())
      {
        return override_root;
      }
    }

    const std::filesystem::path cwd = std::filesystem::current_path();
    return FindProjectRootFromPath(cwd);
  }

  std::filesystem::path FindProjectRootFromPath(const std::filesystem::path &start)
  {
    if (start.empty())
    {
      return {};
    }

    std::filesystem::path candidate = std::filesystem::exists(start) && std::filesystem::is_regular_file(start) ? start.parent_path() : start;
    while (!candidate.empty())
    {
      if (std::filesystem::exists(candidate / "CMakeLists.txt") &&
          std::filesystem::exists(candidate / "Editor") &&
          std::filesystem::exists(candidate / "Engine"))
      {
        return candidate;
      }

      const std::filesystem::path parent = candidate.parent_path();
      if (parent == candidate)
      {
        break;
      }
      candidate = parent;
    }
    return {};
  }

  std::filesystem::path GetExecutableDirectory()
  {
    const std::filesystem::path executable_path = ResolveExecutablePathForProcess();
    if (executable_path.empty())
    {
      return {};
    }

    return executable_path.parent_path();
  }

  std::filesystem::path GetInstalledEditorDataDirectory()
  {
    const char *resource_dir_override = std::getenv("MYRIAD_EDITOR_RESOURCE_DIR");
    if (resource_dir_override != nullptr && *resource_dir_override != '\0')
    {
      return std::filesystem::path(resource_dir_override);
    }

    const std::filesystem::path executable_dir = GetExecutableDirectory();
    if (executable_dir.empty())
    {
      return {};
    }

    const std::vector<std::filesystem::path> candidates = {
        executable_dir.parent_path() / "share" / "myriad-editor",
        executable_dir / "share" / "myriad-editor",
        executable_dir.parent_path() / "Resources" / "myriad-editor",
        executable_dir / "myriad-editor",
    };

    for (const auto &candidate : candidates)
    {
      if (!candidate.empty() && std::filesystem::exists(candidate))
      {
        return candidate;
      }
    }

    return {};
  }

  std::vector<CompilerPreset> LoadCompilerPresets(const std::filesystem::path &root)
  {
    std::vector<CompilerPreset> presets;
    if (root.empty())
    {
      return presets;
    }

    const std::filesystem::path kits_path = root / "CMakeKits.json";
    std::ifstream input(kits_path);
    if (!input.is_open())
    {
      return presets;
    }

    std::ostringstream contents_stream;
    contents_stream << input.rdbuf();
    const std::string contents = contents_stream.str();

    std::size_t search_pos = 0;
    while (true)
    {
      const std::size_t object_start = contents.find('{', search_pos);
      if (object_start == std::string::npos)
      {
        break;
      }

      std::size_t cursor = object_start;
      int depth = 0;
      bool in_string = false;
      bool escaped = false;
      while (cursor < contents.size())
      {
        const char ch = contents[cursor];
        if (in_string)
        {
          if (escaped)
          {
            escaped = false;
          }
          else if (ch == '\\')
          {
            escaped = true;
          }
          else if (ch == '"')
          {
            in_string = false;
          }
        }
        else
        {
          if (ch == '"')
          {
            in_string = true;
          }
          else if (ch == '{')
          {
            ++depth;
          }
          else if (ch == '}')
          {
            --depth;
            if (depth == 0)
            {
              const std::string object_text = contents.substr(object_start, cursor - object_start + 1);
              const std::string name = ExtractJsonString(object_text, "name");
              const std::string toolchain_file = ExtractJsonString(object_text, "toolchainFile");
              if (!name.empty())
              {
                CompilerPreset preset;
                preset.name = name;
                preset.toolchain_file = toolchain_file;
                presets.push_back(preset);
              }
              search_pos = cursor + 1;
              break;
            }
          }
        }
        ++cursor;
      }

      if (cursor >= contents.size())
      {
        break;
      }
    }

    return presets;
  }

  std::filesystem::path ResolvePath(const std::filesystem::path &root, const std::filesystem::path &path)
  {
    if (path.empty())
    {
      return {};
    }

    if (path.is_absolute())
    {
      return path;
    }

    return (root / path).lexically_normal();
  }

  std::filesystem::path FindMatchingBuildDirectory(const std::filesystem::path &root, const std::string &preset_name)
  {
    const std::filesystem::path build_root = root / "build";
    if (!std::filesystem::exists(build_root))
    {
      return {};
    }

    std::filesystem::path best_match;
    std::filesystem::path best_match_with_preset;
    const std::string normalized_preset_name = Lowercase(preset_name);
    for (const auto &entry : std::filesystem::recursive_directory_iterator(build_root))
    {
      if (!entry.is_directory())
      {
        continue;
      }

      const auto cache = entry.path() / "CMakeCache.txt";
      const auto makefile = entry.path() / "Makefile";
      if (!std::filesystem::exists(cache) || !std::filesystem::exists(makefile))
      {
        continue;
      }

      const std::string dir_name = entry.path().filename().string();
      const std::string path_text = entry.path().generic_string();
      const std::string lower_dir_name = Lowercase(dir_name);
      const std::string lower_path_text = Lowercase(path_text);
      const bool has_preset = !normalized_preset_name.empty() &&
                              (lower_path_text.find(normalized_preset_name) != std::string::npos ||
                               lower_dir_name.find(normalized_preset_name) != std::string::npos);

      if (best_match.empty())
      {
        best_match = entry.path();
      }

      if (has_preset)
      {
        best_match_with_preset = entry.path();
        break;
      }
    }

    if (!normalized_preset_name.empty())
    {
      return best_match_with_preset;
    }

    return best_match;
  }

  std::filesystem::path ResolveExecutablePath(const std::filesystem::path &build_dir, const std::filesystem::path &project_root, const std::filesystem::path &path)
  {
    if (path.empty())
    {
      return {};
    }

    const std::filesystem::path absolute_path = path.is_absolute() ? path : (build_dir / path);
    if (!absolute_path.empty() && std::filesystem::exists(absolute_path))
    {
      return absolute_path;
    }

    const std::filesystem::path root_relative = project_root / path;
    if (!root_relative.empty() && std::filesystem::exists(root_relative))
    {
      return root_relative;
    }

    return path;
  }

  std::filesystem::path FindGameExecutable(const std::filesystem::path &build_dir, const std::filesystem::path &override, const std::filesystem::path &project_root)
  {
    const std::filesystem::path resolved_override = ResolveExecutablePath(build_dir, project_root, override);
    if (!resolved_override.empty() && std::filesystem::exists(resolved_override))
    {
      return resolved_override;
    }

    if (build_dir.empty() || !std::filesystem::exists(build_dir))
    {
      return {};
    }

    const std::vector<std::string> names = {"TestECS", "TestECS.exe", "testecs", "testecs.exe"};
    const std::vector<std::filesystem::path> common_locations = {
        build_dir / "Examples" / "TestECS",
        build_dir / "Editor",
        build_dir / "bin",
        build_dir,
    };

    for (const auto &location : common_locations)
    {
      if (!std::filesystem::exists(location))
      {
        continue;
      }

      for (const auto &entry : std::filesystem::directory_iterator(location))
      {
        if (!entry.is_regular_file())
        {
          continue;
        }

        const std::string filename = entry.path().filename().string();
        const std::string lower_name = filename;
        std::string candidate_lower;
        candidate_lower.reserve(lower_name.size());
        std::transform(lower_name.begin(), lower_name.end(), std::back_inserter(candidate_lower), [](unsigned char ch)
                       { return static_cast<char>(std::tolower(ch)); });

        const bool is_target = std::find_if(names.begin(), names.end(), [&](const std::string &name)
                                            {
                               std::string lower_target;
                               lower_target.reserve(name.size());
                               std::transform(name.begin(), name.end(), std::back_inserter(lower_target), [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
                               return candidate_lower == lower_target; }) != names.end();
        if (is_target)
        {
          return entry.path();
        }
      }
    }

    for (const auto &entry : std::filesystem::recursive_directory_iterator(build_dir))
    {
      if (!entry.is_regular_file())
      {
        continue;
      }

      const std::string filename = entry.path().filename().string();
      const std::string lower_name = filename;
      std::string candidate_lower;
      candidate_lower.reserve(lower_name.size());
      std::transform(lower_name.begin(), lower_name.end(), std::back_inserter(candidate_lower), [](unsigned char ch)
                     { return static_cast<char>(std::tolower(ch)); });

      const bool is_target = std::find_if(names.begin(), names.end(), [&](const std::string &name)
                                          {
                               std::string lower_target;
                               lower_target.reserve(name.size());
                               std::transform(name.begin(), name.end(), std::back_inserter(lower_target), [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
                               return candidate_lower == lower_target; }) != names.end();
      if (is_target)
      {
        return entry.path();
      }
    }
    return {};
  }
} // namespace Editor
