#include "project_paths.h"

#include "json_utils.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace Editor
{
  std::string ExpandBuildCommandTemplate(std::string command,
                                         const std::filesystem::path &project_root,
                                         const std::filesystem::path &build_dir,
                                         const CompilerPreset &preset,
                                         const std::filesystem::path &toolchain_path)
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
    return command;
  }

  std::filesystem::path FindProjectRoot()
  {
    std::filesystem::path cwd = std::filesystem::current_path();
    for (auto candidate = cwd; !candidate.empty(); candidate = candidate.parent_path())
    {
      if (std::filesystem::exists(candidate / "CMakeLists.txt") &&
          std::filesystem::exists(candidate / "Editor") &&
          std::filesystem::exists(candidate / "Engine"))
      {
        return candidate;
      }
    }
    return cwd;
  }

  std::vector<CompilerPreset> LoadCompilerPresets(const std::filesystem::path &root)
  {
    std::vector<CompilerPreset> presets;
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
