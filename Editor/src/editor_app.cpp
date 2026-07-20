#include "editor_app.h"

#include "build_bridge.h"
#include "json_utils.h"
#include "process_utils.h"
#include "project_paths.h"

#include "imgui.h"
#ifdef IMGUI_HAS_DOCK
#include "imgui_internal.h"
#endif
#include "rlImGui.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <ctime>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace
{
  constexpr const char *kBuildSpinnerFrames[] = {"|", "/", "-", "\\"};
  constexpr int kBuildSpinnerFrameCount = sizeof(kBuildSpinnerFrames) / sizeof(kBuildSpinnerFrames[0]);

  bool TryExtractBuildProgressPercent(const std::string &line, int &percent)
  {
    for (std::size_t i = 0; i < line.size(); ++i)
    {
      if (line[i] != '%')
      {
        continue;
      }

      std::size_t number_end = i;
      std::size_t number_start = number_end;
      while (number_start > 0 && std::isdigit(static_cast<unsigned char>(line[number_start - 1])) != 0)
      {
        --number_start;
      }

      if (number_start == number_end)
      {
        continue;
      }

      std::size_t bracket_probe = number_start;
      while (bracket_probe > 0 && std::isspace(static_cast<unsigned char>(line[bracket_probe - 1])) != 0)
      {
        --bracket_probe;
      }

      const bool looks_like_cmake_progress = bracket_probe > 0 && line[bracket_probe - 1] == '[';
      if (!looks_like_cmake_progress)
      {
        continue;
      }

      int parsed_percent = std::atoi(line.substr(number_start, number_end - number_start).c_str());
      parsed_percent = std::max(0, std::min(100, parsed_percent));
      percent = parsed_percent;
      return true;
    }

    return false;
  }

  int ThemePresetIndexFromName(const std::string &theme_name, const std::vector<Editor::ThemePreset> &theme_presets)
  {
    for (int i = 0; i < static_cast<int>(theme_presets.size()); ++i)
    {
      if (theme_name == theme_presets[i].name)
      {
        return i;
      }
    }

    return 0;
  }

  std::vector<Editor::ThemePreset> DefaultThemePresets()
  {
    return {
        {"Dark", "dark", "#1A1B1F", "#3E4A5C", "#4B5C74", "#415069", "#53709A", "#2A2F3B", "#1B2230", "#263248"},
        {"Light", "light", "#F4F5F8", "#9CB2CE", "#AFC4DE", "#8FAED1", "#7CA0CA", "#E7EBF2", "#D8DFEB", "#B8C8DE"},
        {"Classic", "classic", "#2C2C2C", "#5A6D7D", "#708697", "#556979", "#6A8296", "#3A3A3A", "#353535", "#454545"},
        {"Cherry", "dark", "#21141A", "#8D3351", "#A44767", "#914061", "#B05676", "#341B28", "#3A1A29", "#5B243D"},
        {"Ocean", "dark", "#0B1B24", "#1B5E73", "#297A93", "#1E6A82", "#2E89A6", "#123141", "#11384A", "#1A5871"},
        {"Solar Flare", "dark", "#1D130C", "#B25B1E", "#CF7430", "#BA6124", "#E7873B", "#2B1B10", "#3A2415", "#5A371F"},
        {"Neon Synth", "dark", "#120E1C", "#7A3CC9", "#9D57EA", "#8A49DA", "#B768FF", "#241935", "#2D1E44", "#4A2C70"},
        {"Emerald City", "dark", "#0D1E17", "#2F8A63", "#46A97C", "#369A70", "#58C18F", "#173227", "#1C3F30", "#286047"},
    };
  }

  std::filesystem::path ResolveEditorResourcePath(const std::filesystem::path &project_root,
                                                  const std::string &filename)
  {
    std::vector<std::filesystem::path> candidates = {
        project_root / "Editor" / filename,
        project_root / filename,
        project_root.parent_path() / "share" / "myriad-editor" / filename,
    };

    const std::filesystem::path installed_data_dir = Editor::GetInstalledEditorDataDirectory();
    if (!installed_data_dir.empty())
    {
      candidates.push_back(installed_data_dir / filename);
    }

    for (const auto &candidate : candidates)
    {
      if (!candidate.empty() && std::filesystem::exists(candidate))
      {
        return candidate;
      }
    }

    return {};
  }

  std::filesystem::path ResolveConfiguredProjectDirectory(const std::string &project_root_text)
  {
    const std::string trimmed_path = Editor::Trim(project_root_text);
    if (trimmed_path.empty())
    {
      return {};
    }

    const std::filesystem::path configured_path(trimmed_path);
    const std::filesystem::path repo_root = Editor::FindProjectRootFromPath(configured_path);
    if (!repo_root.empty())
    {
      return repo_root;
    }

    if (std::filesystem::exists(configured_path) && std::filesystem::is_directory(configured_path))
    {
      return std::filesystem::absolute(configured_path);
    }

    return {};
  }

  std::string FallbackDistributionHeaderSearchDirs()
  {
    return (std::filesystem::path("dist") / "myriad" / "include").generic_string();
  }

  std::string FallbackDistributionLibrarySearchDirs(const std::string &compiler_toolkit, const std::string &build_type)
  {
    const std::string safe_toolkit = compiler_toolkit.empty() ? std::string{"Default"} : compiler_toolkit;
    const std::string safe_build_type = build_type.empty() ? std::string{"Debug"} : build_type;
    return (std::filesystem::path("dist") / "myriad" / safe_toolkit / safe_build_type / "lib").generic_string();
  }

  bool ApplySearchDirDefaults(Editor::EditorSettings &settings,
                              const std::string &header_default,
                              const std::string &library_default,
                              const std::string &previous_library_default = {})
  {
    bool changed = false;
    if (!header_default.empty() && Editor::Trim(settings.header_search_dirs).empty())
    {
      settings.header_search_dirs = header_default;
      changed = true;
    }

    const std::string trimmed_library_dirs = Editor::Trim(settings.library_search_dirs);
    const bool can_replace_library_default = trimmed_library_dirs.empty() || (!previous_library_default.empty() && trimmed_library_dirs == previous_library_default);
    if (!library_default.empty() && can_replace_library_default && settings.library_search_dirs != library_default)
    {
      settings.library_search_dirs = library_default;
      changed = true;
    }

    return changed;
  }

  std::filesystem::path SelectSourceProjectRoot(const std::string &target_root_text, const std::string &display_root_text)
  {
    if (!display_root_text.empty())
    {
      return std::filesystem::path(display_root_text);
    }

    return std::filesystem::path(target_root_text);
  }

  std::string JoinSourceMountPath(const std::string &mount_source_path, const std::string &relative_path)
  {
    std::string root = Editor::Trim(mount_source_path);
    std::replace(root.begin(), root.end(), '\\', '/');
    while (root.size() > 1 && root.back() == '/')
    {
      root.pop_back();
    }

    std::string relative = Editor::Trim(relative_path);
    std::replace(relative.begin(), relative.end(), '\\', '/');
    while (!relative.empty() && relative.front() == '/')
    {
      relative.erase(relative.begin());
    }
    while (!relative.empty() && relative.back() == '/')
    {
      relative.pop_back();
    }

    if (relative.empty())
    {
      return root;
    }
    if (root.empty() || root == "/")
    {
      return "/" + relative;
    }
    return root + "/" + relative;
  }

  std::string DisplayPathToMountRelative(const std::string &mount_source_path, const std::string &display_path, const std::string &fallback_relative_path)
  {
    std::string root = Editor::Trim(mount_source_path);
    std::replace(root.begin(), root.end(), '\\', '/');
    while (root.size() > 1 && root.back() == '/')
    {
      root.pop_back();
    }

    std::string path = Editor::Trim(display_path);
    std::replace(path.begin(), path.end(), '\\', '/');
    while (path.size() > 1 && path.back() == '/')
    {
      path.pop_back();
    }

    if (!root.empty() && path == root)
    {
      return {};
    }
    if (!root.empty() && path.rfind(root + "/", 0) == 0)
    {
      return path.substr(root.size() + 1);
    }
    if (!path.empty() && path.front() != '/')
    {
      return path;
    }
    return fallback_relative_path;
  }

  std::string ProjectRelativePathText(const std::filesystem::path &project_root, const std::filesystem::path &path)
  {
    if (path.empty())
    {
      return {};
    }

    std::filesystem::path resolved_path = path;
    if (!resolved_path.is_absolute() || project_root.empty())
    {
      return resolved_path.generic_string();
    }

    std::error_code error_code;
    const std::filesystem::path relative_path = std::filesystem::relative(resolved_path, project_root, error_code);
    if (!error_code && !relative_path.empty())
    {
      const std::string relative_text = relative_path.generic_string();
      if (relative_text != "." && relative_text != ".." && relative_text.rfind("../", 0) != 0)
      {
        return relative_text;
      }
      if (relative_text == ".")
      {
        return {};
      }
    }

    return resolved_path.generic_string();
  }

  std::filesystem::path ResolveProjectPathText(const std::filesystem::path &project_root, const std::string &path_text)
  {
    const std::string trimmed_path = Editor::Trim(path_text);
    if (trimmed_path.empty())
    {
      return project_root;
    }

    const std::filesystem::path path(trimmed_path);
    if (path.is_absolute())
    {
      return path;
    }
    return project_root / path;
  }

  std::filesystem::path ResolveConfiguredResourcesDirectory(const std::filesystem::path &project_root, const std::string &resources_directory)
  {
    const std::string trimmed_path = Editor::Trim(resources_directory);
    if (trimmed_path.empty())
    {
      return {};
    }
    return ResolveProjectPathText(project_root, trimmed_path);
  }

  std::string NormalizeProjectRelativePathText(const std::filesystem::path &project_root, const std::string &path_text)
  {
    const std::string trimmed_path = Editor::Trim(path_text);
    if (trimmed_path.empty())
    {
      return {};
    }

    return ProjectRelativePathText(project_root, ResolveProjectPathText(project_root, trimmed_path));
  }

  Editor::BuildProfile CaptureBuildProfileFromSettings(const Editor::EditorSettings &settings, const std::string &last_executable_path)
  {
    Editor::BuildProfile profile;
    profile.name = settings.selected_build_profile.empty() ? std::string{"Default"} : settings.selected_build_profile;
    profile.build_mode = settings.build_mode;
    profile.build_socket_host = settings.build_socket_host;
    profile.build_socket_hosts = settings.build_socket_hosts;
    profile.build_socket_port = settings.build_socket_port;
    profile.build_bridge_probe_interval_seconds = settings.build_bridge_probe_interval_seconds;
    profile.build_command_template = settings.build_command_template;
    profile.project_name = settings.project_name;
    profile.compiler_toolkit = settings.compiler_toolkit;
    profile.build_type = settings.build_type;
    profile.target_executable_name = settings.target_executable_name;
    profile.source_directory = settings.source_directory;
    profile.resources_directory = settings.resources_directory;
    profile.build_directory = settings.build_directory;
    profile.header_search_dirs = settings.header_search_dirs;
    profile.library_search_dirs = settings.library_search_dirs;
    profile.export_directory = settings.export_directory;
    profile.last_executable_path = last_executable_path;
    return profile;
  }

  void ApplyBuildProfileToSettings(const Editor::BuildProfile &profile, Editor::EditorSettings &settings)
  {
    settings.selected_build_profile = profile.name.empty() ? std::string{"Default"} : profile.name;
    settings.build_mode = profile.build_mode;
    settings.build_socket_host = profile.build_socket_host;
    settings.build_socket_hosts = profile.build_socket_hosts;
    settings.build_socket_port = profile.build_socket_port;
    settings.build_bridge_probe_interval_seconds = profile.build_bridge_probe_interval_seconds;
    settings.build_command_template = profile.build_command_template;
    settings.project_name = profile.project_name.empty() ? std::string{"TestECS"} : profile.project_name;
    settings.compiler_toolkit = profile.compiler_toolkit;
    settings.build_type = profile.build_type.empty() ? std::string{"Debug"} : profile.build_type;
    settings.target_executable_name = profile.target_executable_name.empty() ? std::string{"TestECS"} : profile.target_executable_name;
    settings.source_directory = profile.source_directory;
    settings.resources_directory = profile.resources_directory;
    settings.build_directory = profile.build_directory;
    settings.last_build_dir = settings.build_directory;
    settings.header_search_dirs = profile.header_search_dirs;
    settings.library_search_dirs = profile.library_search_dirs;
    settings.export_directory = profile.export_directory;
    settings.last_executable_path = profile.last_executable_path;
  }

  int BuildProfileIndexByName(const std::vector<Editor::BuildProfile> &profiles, const std::string &name)
  {
    for (int i = 0; i < static_cast<int>(profiles.size()); ++i)
    {
      if (profiles[i].name == name)
      {
        return i;
      }
    }
    return -1;
  }

  class ScopedCurrentPath
  {
  public:
    explicit ScopedCurrentPath(const std::filesystem::path &path)
    {
      if (path.empty())
      {
        return;
      }

      std::error_code error_code;
      if (!std::filesystem::exists(path, error_code) || !std::filesystem::is_directory(path, error_code))
      {
        return;
      }

      original_path_ = std::filesystem::current_path(error_code);
      if (error_code)
      {
        original_path_.clear();
        return;
      }

      std::filesystem::current_path(path, error_code);
      if (!error_code)
      {
        changed_ = true;
      }
    }

    ~ScopedCurrentPath()
    {
      if (!changed_ || original_path_.empty())
      {
        return;
      }

      std::error_code error_code;
      std::filesystem::current_path(original_path_, error_code);
    }

    ScopedCurrentPath(const ScopedCurrentPath &) = delete;
    ScopedCurrentPath &operator=(const ScopedCurrentPath &) = delete;

  private:
    std::filesystem::path original_path_;
    bool changed_ = false;
  };

  std::string NormalizeProjectBrowserRelativePath(const std::string &path_text)
  {
    std::string sanitized = Editor::Trim(path_text);
    std::replace(sanitized.begin(), sanitized.end(), '\\', '/');
    while (!sanitized.empty() && sanitized.front() == '/')
    {
      sanitized.erase(sanitized.begin());
    }

    std::filesystem::path normalized;
    for (const auto &part : std::filesystem::path(sanitized).lexically_normal())
    {
      const std::string part_text = part.generic_string();
      if (part_text.empty() || part_text == ".")
      {
        continue;
      }
      if (part_text == "..")
      {
        return {};
      }
      normalized /= part;
    }
    return normalized.generic_string();
  }

  void AppendUniqueDirectory(std::vector<std::filesystem::path> &directories, const std::filesystem::path &directory)
  {
    if (directory.empty())
    {
      return;
    }

    std::error_code error_code;
    const std::filesystem::path absolute_directory = std::filesystem::absolute(directory, error_code);
    const std::filesystem::path normalized_directory = error_code ? directory.lexically_normal() : absolute_directory.lexically_normal();
    if (std::find(directories.begin(), directories.end(), normalized_directory) == directories.end())
    {
      directories.push_back(normalized_directory);
    }
  }

  std::vector<std::filesystem::path> RuntimeLibraryDirectories(const std::filesystem::path &executable, const std::filesystem::path &build_dir)
  {
    std::vector<std::filesystem::path> directories;
    AppendUniqueDirectory(directories, executable.parent_path());
    AppendUniqueDirectory(directories, build_dir);
    AppendUniqueDirectory(directories, build_dir / "Engine");
    AppendUniqueDirectory(directories, build_dir / ".myriad_bridge_engine");
    AppendUniqueDirectory(directories, build_dir / "lib");
    return directories;
  }

  std::vector<std::string> ExtractJsonObjectArray(const std::string &text, const std::string &key, int max_items)
  {
    std::vector<std::string> objects;
    if (max_items <= 0)
    {
      return objects;
    }

    const std::string pattern = "\"" + key + "\"";
    const std::size_t key_pos = text.find(pattern);
    if (key_pos == std::string::npos)
    {
      return objects;
    }

    const std::size_t value_pos = text.find(':', key_pos + pattern.size());
    if (value_pos == std::string::npos)
    {
      return objects;
    }

    std::size_t cursor = text.find('[', value_pos + 1);
    if (cursor == std::string::npos)
    {
      return objects;
    }

    ++cursor;
    while (cursor < text.size() && static_cast<int>(objects.size()) < max_items)
    {
      while (cursor < text.size() && text[cursor] != '{' && text[cursor] != ']')
      {
        ++cursor;
      }

      if (cursor >= text.size() || text[cursor] == ']')
      {
        break;
      }

      std::size_t object_start = cursor;
      int depth = 0;
      bool in_string = false;
      bool escaped = false;
      while (cursor < text.size())
      {
        const char ch = text[cursor];
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
              objects.push_back(text.substr(object_start, cursor - object_start + 1));
              ++cursor;
              break;
            }
          }
        }
        ++cursor;
      }
    }

    return objects;
  }

  std::vector<Editor::ThemePreset> LoadThemePresets(const std::filesystem::path &project_root)
  {
    const std::filesystem::path themes_path = ResolveEditorResourcePath(project_root, "themes.json");
    std::ifstream input(themes_path);
    if (!input.is_open())
    {
      return DefaultThemePresets();
    }

    std::ostringstream contents_stream;
    contents_stream << input.rdbuf();
    const std::string contents = contents_stream.str();
    const std::vector<std::string> objects = ExtractJsonObjectArray(contents, "themes", 64);

    std::vector<Editor::ThemePreset> themes;
    for (const auto &object_text : objects)
    {
      Editor::ThemePreset preset{};
      preset.name = Editor::ExtractJsonString(object_text, "name");
      preset.base = Editor::ExtractJsonString(object_text, "base");
      preset.window_bg_hex = Editor::ExtractJsonString(object_text, "windowBg");
      preset.header_hex = Editor::ExtractJsonString(object_text, "header");
      preset.header_hovered_hex = Editor::ExtractJsonString(object_text, "headerHovered");
      preset.button_hex = Editor::ExtractJsonString(object_text, "button");
      preset.button_hovered_hex = Editor::ExtractJsonString(object_text, "buttonHovered");
      preset.frame_bg_hex = Editor::ExtractJsonString(object_text, "frameBg");
      preset.title_bg_hex = Editor::ExtractJsonString(object_text, "titleBg");
      preset.title_bg_active_hex = Editor::ExtractJsonString(object_text, "titleBgActive");

      if (preset.name.empty())
      {
        continue;
      }

      if (preset.base.empty())
      {
        preset.base = "dark";
      }

      themes.push_back(preset);
    }

    if (themes.empty())
    {
      return DefaultThemePresets();
    }

    return themes;
  }

  std::vector<Editor::LayoutPreset> DefaultLayoutPresets()
  {
    return {
        {"Editor Default", "balanced", 25, 28, 30},
        {"Preview Focus", "preview", 22, 28, 25},
        {"Build Focus", "build", 35, 25, 34},
        {"Code Focus", "balanced", 18, 22, 24},
        {"Console Focus", "build", 22, 22, 42},
    };
  }

  std::vector<Editor::LayoutPreset> LoadLayoutPresets(const std::filesystem::path &project_root)
  {
    const std::filesystem::path layouts_path = ResolveEditorResourcePath(project_root, "layouts.json");
    std::ifstream input(layouts_path);
    if (!input.is_open())
    {
      return DefaultLayoutPresets();
    }

    std::ostringstream contents_stream;
    contents_stream << input.rdbuf();
    const std::string contents = contents_stream.str();
    const std::vector<std::string> objects = ExtractJsonObjectArray(contents, "layouts", 64);

    std::vector<Editor::LayoutPreset> layouts;
    for (const auto &object_text : objects)
    {
      Editor::LayoutPreset preset{};
      preset.name = Editor::ExtractJsonString(object_text, "name");
      preset.mode = Editor::ExtractJsonString(object_text, "mode");
      preset.left_percent = Editor::ExtractJsonInt(object_text, "leftPercent", preset.left_percent);
      preset.right_percent = Editor::ExtractJsonInt(object_text, "rightPercent", preset.right_percent);
      preset.bottom_percent = Editor::ExtractJsonInt(object_text, "bottomPercent", preset.bottom_percent);

      if (preset.name.empty())
      {
        continue;
      }

      if (preset.mode.empty())
      {
        preset.mode = "balanced";
      }

      preset.left_percent = std::max(5, std::min(60, preset.left_percent));
      preset.right_percent = std::max(5, std::min(60, preset.right_percent));
      preset.bottom_percent = std::max(10, std::min(70, preset.bottom_percent));
      layouts.push_back(preset);
    }

    if (layouts.empty())
    {
      return DefaultLayoutPresets();
    }

    return layouts;
  }

  bool ParseHexColor(const std::string &hex_text, ImVec4 &out_color)
  {
    std::string hex = Editor::Trim(hex_text);
    if (!hex.empty() && hex.front() == '#')
    {
      hex.erase(hex.begin());
    }

    if (hex.size() != 6)
    {
      return false;
    }

    unsigned int rgb = 0;
    if (std::sscanf(hex.c_str(), "%06x", &rgb) != 1)
    {
      return false;
    }

    const float r = static_cast<float>((rgb >> 16) & 0xFF) / 255.0f;
    const float g = static_cast<float>((rgb >> 8) & 0xFF) / 255.0f;
    const float b = static_cast<float>(rgb & 0xFF) / 255.0f;
    out_color = ImVec4(r, g, b, 1.0f);
    return true;
  }

  std::string ToHexColor(const ImVec4 &color)
  {
    const int r = std::max(0, std::min(255, static_cast<int>(color.x * 255.0f + 0.5f)));
    const int g = std::max(0, std::min(255, static_cast<int>(color.y * 255.0f + 0.5f)));
    const int b = std::max(0, std::min(255, static_cast<int>(color.z * 255.0f + 0.5f)));

    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "#%02X%02X%02X", r, g, b);
    return buffer;
  }

  ImVec4 ShiftBrightness(const ImVec4 &color, float delta)
  {
    ImVec4 shifted = color;
    shifted.x = std::max(0.0f, std::min(1.0f, shifted.x + delta));
    shifted.y = std::max(0.0f, std::min(1.0f, shifted.y + delta));
    shifted.z = std::max(0.0f, std::min(1.0f, shifted.z + delta));
    return shifted;
  }

  std::filesystem::path FindHostedLibraryPath(const std::filesystem::path &build_dir, const std::string &target_executable_name)
  {
    const std::string target_name = Editor::Trim(target_executable_name).empty() ? std::string{"TestECS"} : Editor::Trim(target_executable_name);
    const std::vector<std::filesystem::path> candidates = {
        build_dir / "Examples" / target_name / ("lib" + target_name + ".so"),
        build_dir / "Examples" / target_name / (target_name + ".dll"),
        build_dir / "Examples" / target_name / ("lib" + target_name + ".dylib"),
        build_dir / ("lib" + target_name + ".so"),
        build_dir / (target_name + ".dll"),
        build_dir / ("lib" + target_name + ".dylib"),
    };

    for (const auto &candidate : candidates)
    {
      if (std::filesystem::exists(candidate))
      {
        return candidate;
      }
    }

    const std::filesystem::path search_root = build_dir;
    if (std::filesystem::exists(search_root))
    {
      for (const auto &entry : std::filesystem::recursive_directory_iterator(search_root))
      {
        if (!entry.is_regular_file())
        {
          continue;
        }

        const std::string filename = entry.path().filename().string();
        if (filename == "lib" + target_name + ".so" || filename == target_name + ".dll" || filename == "lib" + target_name + ".dylib")
        {
          return entry.path();
        }
      }
    }

    return {};
  }

  std::vector<std::string> ExtractJsonStringArray(const std::string &text, const std::string &key, int max_items)
  {
    std::vector<std::string> values;
    if (max_items <= 0)
    {
      return values;
    }

    const std::string pattern = "\"" + key + "\"";
    const std::size_t key_pos = text.find(pattern);
    if (key_pos == std::string::npos)
    {
      return values;
    }

    const std::size_t value_pos = text.find(':', key_pos + pattern.size());
    if (value_pos == std::string::npos)
    {
      return values;
    }

    std::size_t cursor = text.find('[', value_pos + 1);
    if (cursor == std::string::npos)
    {
      return values;
    }

    ++cursor;
    while (cursor < text.size() && static_cast<int>(values.size()) < max_items)
    {
      while (cursor < text.size() && (text[cursor] == ' ' || text[cursor] == '\n' || text[cursor] == '\r' || text[cursor] == '\t' || text[cursor] == ','))
      {
        ++cursor;
      }

      if (cursor >= text.size() || text[cursor] == ']')
      {
        break;
      }

      if (text[cursor] != '"')
      {
        ++cursor;
        continue;
      }

      ++cursor;
      std::string value;
      bool escape = false;
      while (cursor < text.size())
      {
        const char ch = text[cursor++];
        if (escape)
        {
          switch (ch)
          {
          case 'n':
            value.push_back('\n');
            break;
          case 'r':
            value.push_back('\r');
            break;
          case 't':
            value.push_back('\t');
            break;
          default:
            value.push_back(ch);
            break;
          }
          escape = false;
          continue;
        }

        if (ch == '\\')
        {
          escape = true;
          continue;
        }

        if (ch == '"')
        {
          values.push_back(value);
          break;
        }

        value.push_back(ch);
      }
    }

    return values;
  }

  const char *CurrentBuildSpinnerFrame()
  {
    const int frame_index = static_cast<int>(GetTime() * 8.0) % kBuildSpinnerFrameCount;
    return kBuildSpinnerFrames[frame_index];
  }

  std::string TimestampForLogPath()
  {
    const std::time_t now = std::time(nullptr);
    std::tm local_time{};
#ifdef _WIN32
    localtime_s(&local_time, &now);
#else
    localtime_r(&now, &local_time);
#endif

    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y%m%d-%H%M%S", &local_time);
    return std::string(buffer);
  }

  ImVec4 ColorFromBasicAnsiCode(int code)
  {
    switch (code)
    {
    case 30:
    case 90:
      return ImVec4(0.35f, 0.35f, 0.35f, 1.0f);
    case 31:
      return ImVec4(0.88f, 0.32f, 0.32f, 1.0f);
    case 91:
      return ImVec4(1.00f, 0.42f, 0.42f, 1.0f);
    case 32:
      return ImVec4(0.42f, 0.84f, 0.42f, 1.0f);
    case 92:
      return ImVec4(0.56f, 0.96f, 0.56f, 1.0f);
    case 33:
      return ImVec4(0.90f, 0.78f, 0.36f, 1.0f);
    case 93:
      return ImVec4(1.00f, 0.89f, 0.48f, 1.0f);
    case 34:
      return ImVec4(0.40f, 0.62f, 0.92f, 1.0f);
    case 94:
      return ImVec4(0.54f, 0.76f, 1.00f, 1.0f);
    case 35:
      return ImVec4(0.78f, 0.50f, 0.90f, 1.0f);
    case 95:
      return ImVec4(0.92f, 0.66f, 1.00f, 1.0f);
    case 36:
      return ImVec4(0.40f, 0.82f, 0.86f, 1.0f);
    case 96:
      return ImVec4(0.56f, 0.95f, 1.00f, 1.0f);
    case 37:
      return ImVec4(0.85f, 0.85f, 0.85f, 1.0f);
    case 97:
      return ImVec4(1.00f, 1.00f, 1.00f, 1.0f);
    default:
      return ImGui::GetStyleColorVec4(ImGuiCol_Text);
    }
  }

  ImVec4 ColorFromAnsi256(int index)
  {
    index = std::max(0, std::min(255, index));
    if (index < 16)
    {
      static const ImVec4 table[16] = {
          ImVec4(0.00f, 0.00f, 0.00f, 1.0f),
          ImVec4(0.80f, 0.00f, 0.00f, 1.0f),
          ImVec4(0.00f, 0.80f, 0.00f, 1.0f),
          ImVec4(0.80f, 0.80f, 0.00f, 1.0f),
          ImVec4(0.00f, 0.00f, 0.80f, 1.0f),
          ImVec4(0.80f, 0.00f, 0.80f, 1.0f),
          ImVec4(0.00f, 0.80f, 0.80f, 1.0f),
          ImVec4(0.75f, 0.75f, 0.75f, 1.0f),
          ImVec4(0.33f, 0.33f, 0.33f, 1.0f),
          ImVec4(1.00f, 0.33f, 0.33f, 1.0f),
          ImVec4(0.33f, 1.00f, 0.33f, 1.0f),
          ImVec4(1.00f, 1.00f, 0.33f, 1.0f),
          ImVec4(0.33f, 0.33f, 1.00f, 1.0f),
          ImVec4(1.00f, 0.33f, 1.00f, 1.0f),
          ImVec4(0.33f, 1.00f, 1.00f, 1.0f),
          ImVec4(1.00f, 1.00f, 1.00f, 1.0f),
      };
      return table[index];
    }

    if (index >= 232)
    {
      const float gray = static_cast<float>(8 + (index - 232) * 10) / 255.0f;
      return ImVec4(gray, gray, gray, 1.0f);
    }

    const int cube = index - 16;
    const int r = cube / 36;
    const int g = (cube / 6) % 6;
    const int b = cube % 6;
    const auto level = [](int value)
    {
      return value == 0 ? 0.0f : static_cast<float>(55 + value * 40) / 255.0f;
    };
    return ImVec4(level(r), level(g), level(b), 1.0f);
  }

  void RenderAnsiTextLine(const std::string &line)
  {
    struct Segment
    {
      std::string text;
      ImVec4 fg;
      bool has_bg;
      ImVec4 bg;
    };

    const std::string line_lower = Editor::Lowercase(line);
    ImVec4 default_fg = ImGui::GetStyleColorVec4(ImGuiCol_Text);
    if (line_lower.find("error") != std::string::npos || line_lower.find("failed") != std::string::npos || line_lower.find("missing") != std::string::npos)
    {
      default_fg = ImVec4(1.00f, 0.34f, 0.26f, 1.0f);
    }
    else if (line_lower.find("warning") != std::string::npos)
    {
      default_fg = ImVec4(1.00f, 0.74f, 0.24f, 1.0f);
    }

    struct AnsiStyle
    {
      ImVec4 fg;
      bool has_bg = false;
      ImVec4 bg = ImVec4(0.0f, 0.0f, 0.0f, 0.45f);
    };

    std::vector<Segment> segments;
    std::string buffer;
    AnsiStyle style{default_fg, false, ImVec4(0.0f, 0.0f, 0.0f, 0.45f)};

    auto flush_segment = [&]()
    {
      if (buffer.empty())
      {
        return;
      }
      segments.push_back({buffer, style.fg, style.has_bg, style.bg});
      buffer.clear();
    };

    std::size_t i = 0;
    while (i < line.size())
    {
      if (line[i] == '\x1b' && (i + 1) < line.size() && line[i + 1] == '[')
      {
        std::size_t j = i + 2;
        while (j < line.size() && line[j] != 'm')
        {
          ++j;
        }

        if (j < line.size() && line[j] == 'm')
        {
          flush_segment();
          const std::string codes_text = line.substr(i + 2, j - (i + 2));
          std::vector<int> codes;
          if (codes_text.empty())
          {
            codes.push_back(0);
          }
          else
          {
            std::string token;
            for (char ch : codes_text)
            {
              if (ch == ';')
              {
                codes.push_back(token.empty() ? 0 : std::atoi(token.c_str()));
                token.clear();
                continue;
              }
              if (std::isdigit(static_cast<unsigned char>(ch)) != 0)
              {
                token.push_back(ch);
              }
            }
            codes.push_back(token.empty() ? 0 : std::atoi(token.c_str()));
          }

          for (std::size_t code_index = 0; code_index < codes.size(); ++code_index)
          {
            const int code = codes[code_index];
            if (code == 0 || code == 39)
            {
              style.fg = default_fg;
              if (code == 0)
              {
                style.has_bg = false;
              }
            }
            else if (code == 49)
            {
              style.has_bg = false;
            }
            else if ((code >= 30 && code <= 37) || (code >= 90 && code <= 97))
            {
              style.fg = ColorFromBasicAnsiCode(code);
            }
            else if ((code >= 40 && code <= 47) || (code >= 100 && code <= 107))
            {
              const int mapped_code = (code >= 100) ? (code - 100 + 90) : (code - 40 + 30);
              style.bg = ColorFromBasicAnsiCode(mapped_code);
              style.bg.w = 0.40f;
              style.has_bg = true;
            }
            else if ((code == 38 || code == 48) && (code_index + 1) < codes.size())
            {
              const bool is_bg = (code == 48);
              const int mode = codes[++code_index];
              if (mode == 5 && (code_index + 1) < codes.size())
              {
                const ImVec4 palette_color = ColorFromAnsi256(codes[++code_index]);
                if (is_bg)
                {
                  style.bg = palette_color;
                  style.bg.w = 0.40f;
                  style.has_bg = true;
                }
                else
                {
                  style.fg = palette_color;
                }
              }
              else if (mode == 2 && (code_index + 3) < codes.size())
              {
                const int r = std::max(0, std::min(255, codes[++code_index]));
                const int g = std::max(0, std::min(255, codes[++code_index]));
                const int b = std::max(0, std::min(255, codes[++code_index]));
                ImVec4 true_color = ImVec4(static_cast<float>(r) / 255.0f,
                                           static_cast<float>(g) / 255.0f,
                                           static_cast<float>(b) / 255.0f,
                                           1.0f);
                if (is_bg)
                {
                  true_color.w = 0.40f;
                  style.bg = true_color;
                  style.has_bg = true;
                }
                else
                {
                  style.fg = true_color;
                }
              }
            }
          }

          i = j + 1;
          continue;
        }
      }

      buffer.push_back(line[i]);
      ++i;
    }

    flush_segment();
    if (segments.empty())
    {
      ImGui::TextUnformatted("");
      return;
    }

    for (std::size_t segment_index = 0; segment_index < segments.size(); ++segment_index)
    {
      if (segment_index > 0)
      {
        ImGui::SameLine(0.0f, 0.0f);
      }

      if (segments[segment_index].has_bg)
      {
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        const ImVec2 text_pos = ImGui::GetCursorScreenPos();
        const ImVec2 text_size = ImGui::CalcTextSize(segments[segment_index].text.c_str());
        draw_list->AddRectFilled(
            text_pos,
            ImVec2(text_pos.x + text_size.x, text_pos.y + text_size.y),
            ImGui::ColorConvertFloat4ToU32(segments[segment_index].bg));
      }

      ImGui::PushStyleColor(ImGuiCol_Text, segments[segment_index].fg);
      ImGui::TextUnformatted(segments[segment_index].text.c_str());
      ImGui::PopStyleColor();
    }
  }
}

#ifdef IMGUI_HAS_DOCK
namespace
{
  bool ApplyDockLayoutPreset(const ImGuiID dockspace_id, const ImVec2 &dockspace_size, const Editor::LayoutPreset &preset)
  {
    if (dockspace_size.x < 100.0f || dockspace_size.y < 100.0f)
    {
      return false;
    }

    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, dockspace_size);

    ImGuiID dock_main = dockspace_id;
    ImGuiID dock_left = 0;
    ImGuiID dock_bottom = 0;
    ImGuiID dock_center = dock_main;

    const float left_ratio = static_cast<float>(std::max(5, std::min(60, preset.left_percent))) / 100.0f;
    const float right_ratio = static_cast<float>(std::max(5, std::min(60, preset.right_percent))) / 100.0f;
    const float bottom_ratio = static_cast<float>(std::max(10, std::min(70, preset.bottom_percent))) / 100.0f;
    const std::string mode = Editor::Lowercase(preset.mode);

    if (mode == "preview")
    {
      ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Right, right_ratio, nullptr, &dock_main);
      dock_left = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Left, left_ratio, nullptr, &dock_main);
      dock_bottom = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Down, bottom_ratio, nullptr, &dock_main);
      dock_center = dock_main;
    }
    else if (mode == "build")
    {
      dock_left = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Left, left_ratio, nullptr, &dock_main);
      dock_bottom = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Down, bottom_ratio, nullptr, &dock_main);
      ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Right, right_ratio, nullptr, &dock_main);
      dock_center = dock_main;
    }
    else
    {
      dock_left = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Left, left_ratio, nullptr, &dock_main);
      ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Right, right_ratio, nullptr, &dock_main);
      dock_bottom = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Down, bottom_ratio, nullptr, &dock_main);
      dock_center = dock_main;
    }

    ImGui::DockBuilderDockWindow("Project", dock_left != 0 ? dock_left : dock_center);
    ImGui::DockBuilderDockWindow("Console", dock_bottom != 0 ? dock_bottom : dock_center);
    ImGui::DockBuilderDockWindow("Game Log", dock_bottom != 0 ? dock_bottom : dock_center);
    ImGui::DockBuilderDockWindow("Scene", dock_center);
    ImGui::DockBuilderDockWindow("Preview", dock_center);
    ImGui::DockBuilderFinish(dockspace_id);
    return true;
  }
} // namespace
#endif

MyriadEditor::~MyriadEditor()
{
  JoinBridgeBuildThread();
  std::cout << "Destroying MyriadEditor." << std::endl;
  StopGame();
  StopPreviewGame();
  EndPreviewLogCapture();
  UnloadHostedPreviewLibrary();
}

void MyriadEditor::Init(Myriad::GameEngineConfig &config)
{
  std::strncpy(config.window_title, "Myriad Editor", sizeof(config.window_title) - 1);
  config.window_title[sizeof(config.window_title) - 1] = '\0';
  config.framerate = 30;
  config.window_config.vsync = true;
}

void MyriadEditor::Start()
{
  MyrGameApplication::Start();
  auto log_startup_line = [this](const std::string &line)
  {
    const std::string message = "[Editor] " + line;
    std::fprintf(stderr, "%s\n", message.c_str());
    std::fflush(stderr);
    AppendConsoleLine(line);
  };

  model_.project_root = Editor::FindProjectRoot();
  const std::filesystem::path initial_project_root = model_.project_root;
  log_startup_line("Startup working directory: " + std::filesystem::current_path().string());
  log_startup_line("Startup detected project root: " + (model_.project_root.empty() ? std::string{"<none>"} : model_.project_root.string()));
  for (const auto &line : Editor::GetEditorSettingsDiscoveryLog(model_.project_root))
  {
    log_startup_line(line);
  }

  model_.settings = Editor::LoadEditorSettings(model_.project_root);
  const std::filesystem::path configured_project_root = ResolveConfiguredProjectDirectory(model_.settings.project_root_path);
  if (!configured_project_root.empty() && configured_project_root != model_.project_root)
  {
    model_.project_root = configured_project_root;
    log_startup_line("Using configured project directory: " + model_.project_root.string());
  }

  if (configured_project_root.empty())
  {
    const std::string recovery_build_dir = model_.settings.build_directory.empty() ? model_.settings.last_build_dir : model_.settings.build_directory;
    const std::filesystem::path recovered_project_root = Editor::FindProjectRootFromPath(recovery_build_dir.empty() ? std::filesystem::path(model_.settings.last_executable_path) : std::filesystem::path(recovery_build_dir));
    if (!recovered_project_root.empty() && recovered_project_root != model_.project_root)
    {
      model_.project_root = recovered_project_root;
      model_.settings.project_root_path = model_.project_root.string();
      log_startup_line("Recovered project root from editor settings: " + model_.project_root.string());
    }
  }

  if (model_.project_root != initial_project_root)
  {
    for (const auto &line : Editor::GetEditorSettingsDiscoveryLog(model_.project_root))
    {
      log_startup_line(line);
    }
  }

  const std::filesystem::path resolved_themes_path = ResolveEditorResourcePath(model_.project_root, "themes.json");
  const std::filesystem::path resolved_layouts_path = ResolveEditorResourcePath(model_.project_root, "layouts.json");
  const std::filesystem::path resolved_settings_read_path = Editor::GetEditorSettingsReadPath(model_.project_root);
  const std::filesystem::path resolved_settings_write_path = Editor::GetEditorSettingsWritePath(model_.project_root);

  log_startup_line(std::string("Resolved themes path: ") + (resolved_themes_path.empty() ? "<default presets>" : resolved_themes_path.string()));
  log_startup_line(std::string("Resolved layouts path: ") + (resolved_layouts_path.empty() ? "<default presets>" : resolved_layouts_path.string()));
  log_startup_line(std::string("Resolved settings read path: ") + (resolved_settings_read_path.empty() ? "<none>" : resolved_settings_read_path.string()));
  log_startup_line(std::string("Resolved settings write path: ") + (resolved_settings_write_path.empty() ? "<none>" : resolved_settings_write_path.string()));

  if (!IsWindowReady())
  {
    log_startup_line("Raylib window is not ready; editor UI initialization aborted.");
    model_.status = "Editor window failed to initialize.";
    return;
  }

  rlImGuiSetup(true);
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.IniFilename = nullptr;
#ifdef IMGUI_HAS_DOCK
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#endif

  model_.compiler_presets = Editor::LoadCompilerPresets(model_.project_root);
  model_.theme_presets = LoadThemePresets(model_.project_root);
  model_.layout_presets = LoadLayoutPresets(model_.project_root);
  model_.build_command_template = Editor::ResolveBuildCommandTemplate(model_.settings);
  if (model_.compiler_presets.empty())
  {
    model_.compiler_presets.push_back({"Default", {}});
  }

  if (model_.settings.compiler_toolkit.empty())
  {
    model_.settings.compiler_toolkit = model_.settings.last_compiler_preset;
  }
  if (model_.settings.build_type.empty())
  {
    model_.settings.build_type = "Debug";
  }
  if (model_.settings.project_name.empty())
  {
    model_.settings.project_name = "TestECS";
  }
  if (model_.settings.target_executable_name.empty())
  {
    model_.settings.target_executable_name = "TestECS";
  }

  model_.selected_preset_index = 0;
  if (!model_.settings.compiler_toolkit.empty())
  {
    for (int i = 0; i < static_cast<int>(model_.compiler_presets.size()); ++i)
    {
      if (model_.compiler_presets[i].name == model_.settings.compiler_toolkit)
      {
        model_.selected_preset_index = i;
        break;
      }
    }
  }

  if (model_.settings.build_profiles.empty())
  {
    model_.settings.selected_build_profile = model_.settings.selected_build_profile.empty() ? std::string{"Default"} : model_.settings.selected_build_profile;
    model_.settings.build_profiles.push_back(CaptureBuildProfileFromSettings(model_.settings, model_.settings.last_executable_path));
  }
  model_.selected_build_profile_index = BuildProfileIndexByName(model_.settings.build_profiles, model_.settings.selected_build_profile);
  if (model_.selected_build_profile_index < 0)
  {
    model_.selected_build_profile_index = 0;
    ApplyBuildProfileToSettings(model_.settings.build_profiles.front(), model_.settings);
  }

  model_.build_dir_input = NormalizeProjectRelativePathText(model_.project_root, model_.settings.build_directory.empty() ? model_.settings.last_build_dir : model_.settings.build_directory);
  model_.executable_input = model_.settings.last_executable_path;
  model_.selected_theme_preset_index = ThemePresetIndexFromName(model_.settings.theme_preset, model_.theme_presets);

  model_.ui_font_scale = static_cast<float>(model_.settings.ui_font_scale_percent) / 100.0f;
  model_.ui_rounding = static_cast<float>(model_.settings.ui_rounding);
  model_.ui_spacing_density = static_cast<float>(model_.settings.ui_spacing_percent) / 100.0f;
  if (!ParseHexColor(model_.settings.ui_accent_hex, model_.ui_accent_color))
  {
    model_.ui_accent_color = ImVec4(0.30f, 0.54f, 0.81f, 1.0f);
  }

  ApplyThemePresetByIndex(model_.selected_theme_preset_index);
  ApplyThemeCustomizations();
#ifdef IMGUI_HAS_DOCK
  if (model_.layout_presets.empty())
  {
    model_.layout_presets = DefaultLayoutPresets();
  }
  model_.selected_layout_preset_index = std::max(0, std::min(model_.settings.layout_preset_index, static_cast<int>(model_.layout_presets.size()) - 1));
#else
  model_.selected_layout_preset_index = 0;
#endif
  model_.show_project_window = model_.settings.panel_build_workflow_open;
  model_.show_scene_window = model_.settings.panel_scene_open;
  model_.show_preview_window = model_.settings.panel_preview_open;
  model_.show_console_window = model_.settings.panel_console_open;
  model_.show_game_log_window = model_.settings.panel_game_log_open;
  if (!model_.show_project_window && !model_.show_scene_window && !model_.show_preview_window && !model_.show_console_window && !model_.show_game_log_window)
  {
    model_.show_project_window = true;
    model_.show_scene_window = true;
    model_.show_preview_window = true;
    model_.show_console_window = true;
  }
  model_.dock_layout_apply_requested = true;
  model_.build_bridge_last_probe_time = GetTime();
  RefreshBuildOptions();
  RefreshPaths(false);
  if (!model_.settings.project_mount_path.empty())
  {
    const std::filesystem::path mounted_project_path(model_.settings.project_mount_path);
    const std::filesystem::path browser_start_path = mounted_project_path.parent_path();
    model_.project_browser_relative_path = browser_start_path == "." ? std::string{} : browser_start_path.generic_string();
  }
  RefreshProjectBrowser(model_.project_browser_relative_path);
  model_.last_selected_preset_index = model_.selected_preset_index;

  if (model_.settings.editor_window_width > 0 && model_.settings.editor_window_height > 0)
  {
    SetWindowSize(model_.settings.editor_window_width, model_.settings.editor_window_height);
  }

  model_.settings.compiler_toolkit = model_.compiler_presets[model_.selected_preset_index].name;
  model_.settings.last_compiler_preset = model_.settings.compiler_toolkit;
  model_.settings.build_directory = ProjectRelativePathText(model_.project_root, model_.build_dir);
  model_.settings.last_build_dir = model_.settings.build_directory;
  model_.settings.last_executable_path = model_.executable_input;
  model_.settings.layout_preset_index = model_.selected_layout_preset_index;
  if (model_.default_header_search_dirs.empty())
  {
    model_.default_header_search_dirs = FallbackDistributionHeaderSearchDirs();
  }
  if (model_.default_library_search_dirs.empty())
  {
    model_.default_library_search_dirs = FallbackDistributionLibrarySearchDirs(model_.settings.compiler_toolkit, model_.settings.build_type);
  }
  if (ApplySearchDirDefaults(model_.settings, model_.default_header_search_dirs, model_.default_library_search_dirs))
  {
    model_.project_settings_dirty = true;
  }
  model_.status = "Ready. Choose a compiler toolkit and build type, then build and run the selected game project.";
}

void MyriadEditor::Render()
{
  ClearBackground({0, 0, 0, 255});

  auto mark_settings_dirty = [this]()
  {
    model_.project_settings_dirty = true;
  };

  const int current_window_width = GetScreenWidth();
  const int current_window_height = GetScreenHeight();
  if (model_.settings.editor_window_width != current_window_width || model_.settings.editor_window_height != current_window_height)
  {
    model_.settings.editor_window_width = current_window_width;
    model_.settings.editor_window_height = current_window_height;
    mark_settings_dirty();
  }

  if (model_.preview_stop_requested)
  {
    StopPreviewGame();
    EndPreviewLogCapture();
    model_.preview_stop_requested = false;
  }
  else if (model_.preview_texture_cleanup_requested)
  {
    model_.preview_texture = {};
    model_.preview_texture_ready = false;
    model_.preview_texture_cleanup_requested = false;
  }

  PumpGameLogOutput();

  rlImGuiBegin();

#ifdef IMGUI_HAS_DOCK
  {
    ImGuiWindowFlags dockspace_window_flags = ImGuiWindowFlags_NoDocking;
    dockspace_window_flags |= ImGuiWindowFlags_NoTitleBar;
    dockspace_window_flags |= ImGuiWindowFlags_NoCollapse;
    dockspace_window_flags |= ImGuiWindowFlags_NoResize;
    dockspace_window_flags |= ImGuiWindowFlags_NoMove;
    dockspace_window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    dockspace_window_flags |= ImGuiWindowFlags_NoNavFocus;

    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImVec2 dockspace_pos = viewport->Pos;
    ImVec2 dockspace_size = viewport->Size;
    const float menu_bar_height = ImGui::GetFrameHeight();
    dockspace_pos.y += menu_bar_height;
    dockspace_size.y = std::max(1.0f, dockspace_size.y - menu_bar_height);

    ImGui::SetNextWindowPos(dockspace_pos);
    ImGui::SetNextWindowSize(dockspace_size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("MyriadDockSpaceHost", nullptr, dockspace_window_flags);
    ImGui::PopStyleVar(2);

    const ImGuiID dockspace_id = ImGui::GetID("MyriadDockSpace");
    const float status_bar_height = ImGui::GetFrameHeightWithSpacing() + 6.0f;
    const ImVec2 dockspace_available_size = ImGui::GetContentRegionAvail();
    const ImVec2 dockspace_layout_size = ImVec2(std::max(1.0f, dockspace_available_size.x), std::max(1.0f, dockspace_available_size.y - status_bar_height));
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, -status_bar_height), ImGuiDockNodeFlags_None);

    if (model_.dock_layout_apply_requested)
    {
      model_.show_project_window = true;
      model_.show_scene_window = true;
      model_.show_preview_window = true;
      model_.show_console_window = true;
      model_.show_game_log_window = true;
      model_.show_project_window = true;
      if (model_.layout_presets.empty())
      {
        model_.layout_presets = DefaultLayoutPresets();
      }
      const int safe_layout_index = std::max(0, std::min(model_.selected_layout_preset_index, static_cast<int>(model_.layout_presets.size()) - 1));
      model_.dock_layout_apply_requested = !ApplyDockLayoutPreset(dockspace_id, dockspace_layout_size, model_.layout_presets[safe_layout_index]);
    }

    ImGui::SetCursorPos(ImVec2(0.0f, ImGui::GetWindowHeight() - status_bar_height));
    ImGui::Separator();
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(18.0f, 4.0f));
    ImGui::BeginChild("BottomStatusBar", ImVec2(0.0f, status_bar_height - 2.0f), false, ImGuiWindowFlags_NoScrollbar);
    const int safe_preset_index = std::max(0, std::min(model_.selected_preset_index, static_cast<int>(model_.compiler_presets.size()) - 1));
    const std::string selected_toolchain_label = model_.compiler_presets.empty() ? std::string{"Default"} : model_.compiler_presets[safe_preset_index].name;
    const std::string selected_build_type_label = model_.settings.build_type.empty() ? std::string{"Debug"} : model_.settings.build_type;
    const std::string build_selector_label = "Toolchain: " + selected_toolchain_label + " / " + selected_build_type_label;
    if (Editor::ShouldUseSocketBuilds(model_.settings))
    {
      ImGui::TextUnformatted("Build Bridge:");
      ImGui::SameLine();
      const ImVec4 bridge_color = model_.build_bridge_connected ? ImVec4(0.20f, 0.78f, 0.34f, 1.0f) : ImVec4(0.88f, 0.34f, 0.25f, 1.0f);
      ImGui::TextColored(bridge_color, "%s", model_.build_bridge_connected ? "Connected" : "Disconnected");

      ImGui::SameLine();
      ImGui::TextDisabled("|");
      ImGui::SameLine();
      if (ImGui::SmallButton("Test"))
      {
        RefreshBuildBridgeStatus(true);
      }

      ImGui::SameLine();
      ImGui::TextDisabled("|");
      ImGui::SameLine();
      ImGui::TextUnformatted("Host");
      ImGui::SameLine();
      ImGui::SetNextItemWidth(160.0f);
      char bridge_host_buffer[256];
      std::strncpy(bridge_host_buffer, model_.settings.build_socket_host.c_str(), sizeof(bridge_host_buffer) - 1);
      bridge_host_buffer[sizeof(bridge_host_buffer) - 1] = '\0';
      if (ImGui::InputText("##BridgeHost", bridge_host_buffer, sizeof(bridge_host_buffer)))
      {
        const std::string normalized_host = Editor::Trim(std::string(bridge_host_buffer));
        model_.settings.build_socket_host = normalized_host;
        if (!normalized_host.empty() && std::find(model_.settings.build_socket_hosts.begin(), model_.settings.build_socket_hosts.end(), normalized_host) == model_.settings.build_socket_hosts.end())
        {
          model_.settings.build_socket_hosts.push_back(normalized_host);
        }
        mark_settings_dirty();
      }

      ImGui::SameLine();
      ImGui::TextDisabled("|");
      ImGui::SameLine();
      ImGui::TextUnformatted("Port");
      ImGui::SameLine();
      ImGui::SetNextItemWidth(90.0f);
      char bridge_port_buffer[16];
      std::snprintf(bridge_port_buffer, sizeof(bridge_port_buffer), "%d", model_.settings.build_socket_port);
      if (ImGui::InputText("##BridgePort", bridge_port_buffer, sizeof(bridge_port_buffer), ImGuiInputTextFlags_CharsDecimal))
      {
        const std::string port_text = Editor::Trim(std::string(bridge_port_buffer));
        if (!port_text.empty())
        {
          const int parsed_port = std::atoi(port_text.c_str());
          const int clamped_port = std::max(1, std::min(65535, parsed_port));
          if (clamped_port != model_.settings.build_socket_port)
          {
            model_.settings.build_socket_port = clamped_port;
            mark_settings_dirty();
          }
        }
      }

      if (model_.bridge_build_in_progress)
      {
        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.95f, 0.80f, 0.25f, 1.0f), "%s Building", CurrentBuildSpinnerFrame());
        if (model_.bridge_build_progress_percent >= 0)
        {
          ImGui::SameLine();
          ImGui::TextDisabled("|");
          ImGui::SameLine();
          const float progress_fraction = static_cast<float>(model_.bridge_build_progress_percent) / 100.0f;
          const std::string progress_overlay = std::to_string(model_.bridge_build_progress_percent) + "%";
          ImGui::ProgressBar(progress_fraction, ImVec2(130.0f, 0.0f), progress_overlay.c_str());
        }
      }
      else if (model_.build_succeeded)
      {
        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.35f, 0.85f, 0.45f, 1.0f), "Build OK");
      }

      if (model_.bridge_rebuild_needed)
      {
        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.95f, 0.75f, 0.25f, 1.0f), "Rebuild needed (%d)", model_.bridge_changed_file_count);
      }

      if (!model_.build_bridge_status_text.empty())
      {
        ImGui::SameLine();
        ImGui::TextDisabled("| %s", model_.build_bridge_status_text.c_str());
      }

      if (model_.build_bridge_warning_active)
      {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.95f, 0.45f, 0.30f, 1.0f), "Warning");
      }

      ImGui::SameLine();
      ImGui::TextDisabled("|");
      ImGui::SameLine();
      if (ImGui::SmallButton(build_selector_label.c_str()))
      {
        model_.show_project_window = true;
      }
    }
    else
    {
      ImGui::TextDisabled("Build bridge inactive (local build mode).");
      ImGui::SameLine();
      ImGui::TextDisabled("|");
      ImGui::SameLine();
      if (ImGui::SmallButton(build_selector_label.c_str()))
      {
        model_.show_project_window = true;
      }
      if (!model_.status.empty())
      {
        ImGui::SameLine();
        ImGui::TextDisabled("| %s", model_.status.c_str());
      }
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();

    ImGui::End();
  }
#endif

  if (ImGui::BeginMainMenuBar())
  {
    if (ImGui::BeginMenu("Project"))
    {
      if (ImGui::MenuItem("New Project"))
      {
        model_.show_new_project_dialog = true;
        model_.show_open_project_dialog = false;
        model_.show_export_directory_dialog = false;
        RefreshProjectBrowser(model_.project_browser_relative_path);
      }
      if (ImGui::MenuItem("Open Project"))
      {
        model_.show_open_project_dialog = true;
        model_.show_new_project_dialog = false;
        model_.show_export_directory_dialog = false;
        RefreshProjectBrowser(model_.project_browser_relative_path);
      }
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Window"))
    {
      ImGui::MenuItem("Project", nullptr, &model_.show_project_window);
      ImGui::MenuItem("Scene", nullptr, &model_.show_scene_window);
      ImGui::MenuItem("Preview", nullptr, &model_.show_preview_window);
      ImGui::MenuItem("Console", nullptr, &model_.show_console_window);
      ImGui::MenuItem("Game Log", nullptr, &model_.show_game_log_window);
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Run"))
    {
      const bool can_start_build = !model_.bridge_build_in_progress;
      const std::string build_menu_label = "Build " + (model_.settings.target_executable_name.empty() ? std::string{"TestECS"} : model_.settings.target_executable_name);
      if (ImGui::MenuItem(build_menu_label.c_str(), nullptr, false, can_start_build))
      {
        model_.run_after_build_request = false;
        BuildTestECS();
      }
      if (ImGui::MenuItem("Build && Run", nullptr, false, can_start_build))
      {
        model_.run_after_build_request = true;
        BuildTestECS();
        if (!Editor::ShouldUseSocketBuilds(model_.settings) && model_.build_succeeded)
        {
          model_.run_after_build_request = false;
          RunTestECS();
        }
      }
      if (ImGui::MenuItem("Stop"))
      {
        StopGame();
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }

  if (model_.selected_preset_index != model_.last_selected_preset_index)
  {
    const std::string previous_library_default = model_.default_library_search_dirs;
    model_.last_selected_preset_index = model_.selected_preset_index;
    model_.settings.compiler_toolkit = model_.compiler_presets[model_.selected_preset_index].name;
    model_.settings.last_compiler_preset = model_.settings.compiler_toolkit;
    RefreshBuildOptions();
    ApplySearchDirDefaults(model_.settings, model_.default_header_search_dirs, model_.default_library_search_dirs, previous_library_default);
    RefreshPaths(true);
    model_.build_dir_input = NormalizeProjectRelativePathText(model_.project_root, model_.build_dir_input);
    model_.settings.build_directory = model_.build_dir_input;
    model_.settings.last_build_dir = model_.settings.build_directory;
    model_.settings.last_executable_path = model_.executable_input;
    mark_settings_dirty();
  }

  if (model_.settings.panel_build_workflow_open != model_.show_project_window ||
      model_.settings.panel_scene_open != model_.show_scene_window ||
      model_.settings.panel_preview_open != model_.show_preview_window ||
      model_.settings.panel_console_open != model_.show_console_window ||
      model_.settings.panel_game_log_open != model_.show_game_log_window)
  {
    mark_settings_dirty();
  }

  PumpBridgeBuildUpdates();
  if (!model_.bridge_build_in_progress)
  {
    RefreshBuildBridgeStatus(false);
  }

  if (model_.show_project_window)
  {
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(model_.settings.panel_build_workflow_width), static_cast<float>(model_.settings.panel_build_workflow_height)), ImGuiCond_FirstUseEver);
    ImGui::Begin("Project", &model_.show_project_window);

    ImGui::Text("Project root: %s", model_.project_root.string().c_str());
    ImGui::SameLine();
    ImGui::TextDisabled("%s", model_.project_settings_dirty ? "Unsaved changes" : "Saved");
    ImGui::Separator();

    if (ImGui::BeginTabBar("ProjectTabs"))
    {
      if (ImGui::BeginTabItem("Build"))
      {
        const std::string build_title = "Build and run " + (model_.settings.target_executable_name.empty() ? std::string{"TestECS"} : model_.settings.target_executable_name);
        ImGui::Text("%s", build_title.c_str());
        ImGui::Separator();

        if (model_.settings.build_profiles.empty())
        {
          model_.settings.selected_build_profile = model_.settings.selected_build_profile.empty() ? std::string{"Default"} : model_.settings.selected_build_profile;
          model_.settings.build_profiles.push_back(CaptureBuildProfileFromSettings(model_.settings, model_.executable_input));
          model_.selected_build_profile_index = 0;
        }

        std::vector<const char *> build_profile_labels;
        build_profile_labels.reserve(model_.settings.build_profiles.size());
        for (const auto &profile : model_.settings.build_profiles)
        {
          build_profile_labels.push_back(profile.name.c_str());
        }
        model_.selected_build_profile_index = std::max(0, std::min(model_.selected_build_profile_index, static_cast<int>(model_.settings.build_profiles.size()) - 1));
        if (ImGui::Combo("Build profile", &model_.selected_build_profile_index, build_profile_labels.data(), static_cast<int>(build_profile_labels.size())))
        {
          const int previous_index = BuildProfileIndexByName(model_.settings.build_profiles, model_.settings.selected_build_profile);
          if (previous_index >= 0)
          {
            model_.settings.build_profiles[previous_index] = CaptureBuildProfileFromSettings(model_.settings, model_.executable_input);
          }

          ApplyBuildProfileToSettings(model_.settings.build_profiles[model_.selected_build_profile_index], model_.settings);
          model_.build_command_template = Editor::ResolveBuildCommandTemplate(model_.settings);
          model_.selected_preset_index = 0;
          for (int i = 0; i < static_cast<int>(model_.compiler_presets.size()); ++i)
          {
            if (model_.compiler_presets[i].name == model_.settings.compiler_toolkit)
            {
              model_.selected_preset_index = i;
              break;
            }
          }
          model_.last_selected_preset_index = model_.selected_preset_index;
          RefreshBuildOptions();
          model_.build_dir_input = NormalizeProjectRelativePathText(model_.project_root, model_.settings.build_directory.empty() ? model_.settings.last_build_dir : model_.settings.build_directory);
          model_.executable_input = model_.settings.last_executable_path;
          RefreshPaths(false);
          RefreshBuildBridgeStatus(true);
          mark_settings_dirty();
        }

        char new_build_profile_name_buffer[256];
        std::strncpy(new_build_profile_name_buffer, model_.new_build_profile_name.c_str(), sizeof(new_build_profile_name_buffer) - 1);
        new_build_profile_name_buffer[sizeof(new_build_profile_name_buffer) - 1] = '\0';
        if (ImGui::InputText("New profile name", new_build_profile_name_buffer, sizeof(new_build_profile_name_buffer)))
        {
          model_.new_build_profile_name = Editor::Trim(std::string(new_build_profile_name_buffer));
        }
        ImGui::SameLine();
        if (ImGui::Button("Create profile"))
        {
          const std::string profile_name = Editor::Trim(model_.new_build_profile_name);
          if (profile_name.empty())
          {
            AppendConsoleLine("Build profile name is required.");
          }
          else if (BuildProfileIndexByName(model_.settings.build_profiles, profile_name) >= 0)
          {
            AppendConsoleLine("Build profile already exists: " + profile_name);
          }
          else
          {
            const int previous_index = BuildProfileIndexByName(model_.settings.build_profiles, model_.settings.selected_build_profile);
            if (previous_index >= 0)
            {
              model_.settings.build_profiles[previous_index] = CaptureBuildProfileFromSettings(model_.settings, model_.executable_input);
            }

            Editor::BuildProfile profile = CaptureBuildProfileFromSettings(model_.settings, model_.executable_input);
            profile.name = profile_name;
            model_.settings.build_profiles.push_back(profile);
            model_.selected_build_profile_index = static_cast<int>(model_.settings.build_profiles.size()) - 1;
            ApplyBuildProfileToSettings(model_.settings.build_profiles[model_.selected_build_profile_index], model_.settings);
            model_.new_build_profile_name.clear();
            mark_settings_dirty();
          }
        }

        ImGui::Separator();

        std::vector<const char *> preset_labels;
        preset_labels.reserve(model_.compiler_presets.size());
        for (const auto &preset : model_.compiler_presets)
        {
          preset_labels.push_back(preset.name.c_str());
        }

        if (ImGui::Combo("Compiler toolkit", &model_.selected_preset_index, preset_labels.data(), static_cast<int>(preset_labels.size())))
        {
          const std::string previous_library_default = model_.default_library_search_dirs;
          model_.last_selected_preset_index = model_.selected_preset_index;
          model_.settings.compiler_toolkit = model_.compiler_presets[model_.selected_preset_index].name;
          model_.settings.last_compiler_preset = model_.settings.compiler_toolkit;
          RefreshBuildOptions();
          ApplySearchDirDefaults(model_.settings, model_.default_header_search_dirs, model_.default_library_search_dirs, previous_library_default);
          RefreshPaths(true);
          model_.build_dir_input = NormalizeProjectRelativePathText(model_.project_root, model_.build_dir_input);
          model_.settings.build_directory = model_.build_dir_input;
          model_.settings.last_build_dir = model_.settings.build_directory;
          model_.settings.last_executable_path = model_.executable_input;
          mark_settings_dirty();
        }

        std::vector<const char *> build_type_labels;
        build_type_labels.reserve(model_.build_type_options.size());
        int selected_build_type_index = 0;
        for (int i = 0; i < static_cast<int>(model_.build_type_options.size()); ++i)
        {
          build_type_labels.push_back(model_.build_type_options[i].c_str());
          if (model_.build_type_options[i] == model_.settings.build_type)
          {
            selected_build_type_index = i;
          }
        }
        if (build_type_labels.empty())
        {
          model_.build_type_options = {"Debug"};
          build_type_labels.push_back(model_.build_type_options.front().c_str());
          selected_build_type_index = 0;
        }

        if (ImGui::Combo("Build type", &selected_build_type_index, build_type_labels.data(), static_cast<int>(build_type_labels.size())))
        {
          const std::string previous_library_default = model_.default_library_search_dirs;
          model_.settings.build_type = model_.build_type_options[selected_build_type_index];
          RefreshBuildOptions();
          ApplySearchDirDefaults(model_.settings, model_.default_header_search_dirs, model_.default_library_search_dirs, previous_library_default);
          RefreshPaths(true);
          model_.build_dir_input = NormalizeProjectRelativePathText(model_.project_root, model_.build_dir_input);
          model_.settings.build_directory = model_.build_dir_input;
          model_.settings.last_build_dir = model_.settings.build_directory;
          model_.settings.last_executable_path = model_.executable_input;
          mark_settings_dirty();
        }

        char project_name_buffer[256];
        std::strncpy(project_name_buffer, model_.settings.project_name.c_str(), sizeof(project_name_buffer) - 1);
        project_name_buffer[sizeof(project_name_buffer) - 1] = '\0';
        if (ImGui::InputText("Project name", project_name_buffer, sizeof(project_name_buffer)))
        {
          model_.settings.project_name = Editor::Trim(std::string(project_name_buffer));
          mark_settings_dirty();
        }

        char target_executable_name_buffer[256];
        std::strncpy(target_executable_name_buffer, model_.settings.target_executable_name.c_str(), sizeof(target_executable_name_buffer) - 1);
        target_executable_name_buffer[sizeof(target_executable_name_buffer) - 1] = '\0';
        if (ImGui::InputText("Target executable", target_executable_name_buffer, sizeof(target_executable_name_buffer)))
        {
          model_.settings.target_executable_name = Editor::Trim(std::string(target_executable_name_buffer));
          mark_settings_dirty();
        }

        char source_directory_buffer[1024];
        std::strncpy(source_directory_buffer, model_.settings.source_directory.c_str(), sizeof(source_directory_buffer) - 1);
        source_directory_buffer[sizeof(source_directory_buffer) - 1] = '\0';
        if (ImGui::InputText("Source directory", source_directory_buffer, sizeof(source_directory_buffer)))
        {
          model_.settings.source_directory = Editor::Trim(std::string(source_directory_buffer));
          mark_settings_dirty();
        }
        ImGui::SameLine();
        if (ImGui::Button("Browse source"))
        {
          model_.source_browser_relative_path = ProjectRelativePathText(model_.project_root, ResolveProjectPathText(model_.project_root, model_.settings.source_directory));
          model_.show_source_directory_dialog = true;
          RefreshSourceDirectoryBrowser(model_.source_browser_relative_path);
        }

        char resources_directory_buffer[1024];
        std::strncpy(resources_directory_buffer, model_.settings.resources_directory.c_str(), sizeof(resources_directory_buffer) - 1);
        resources_directory_buffer[sizeof(resources_directory_buffer) - 1] = '\0';
        if (ImGui::InputText("Resources directory", resources_directory_buffer, sizeof(resources_directory_buffer)))
        {
          model_.settings.resources_directory = Editor::Trim(std::string(resources_directory_buffer));
          mark_settings_dirty();
        }
        ImGui::SameLine();
        if (ImGui::Button("Browse resources"))
        {
          model_.resources_browser_relative_path = ProjectRelativePathText(model_.project_root, ResolveProjectPathText(model_.project_root, model_.settings.resources_directory));
          model_.show_resources_directory_dialog = true;
          RefreshResourcesDirectoryBrowser(model_.resources_browser_relative_path);
        }

        char project_root_buffer[1024];
        std::strncpy(project_root_buffer, model_.settings.project_root_path.c_str(), sizeof(project_root_buffer) - 1);
        project_root_buffer[sizeof(project_root_buffer) - 1] = '\0';
        if (ImGui::InputText("Project directory", project_root_buffer, sizeof(project_root_buffer)))
        {
          model_.settings.project_root_path = Editor::Trim(std::string(project_root_buffer));
          mark_settings_dirty();
        }

        if (ImGui::Button("Apply project directory"))
        {
          const std::filesystem::path configured_project_root = ResolveConfiguredProjectDirectory(model_.settings.project_root_path);
          if (!configured_project_root.empty())
          {
            ApplyProjectDirectory(configured_project_root, model_.settings.project_mount_path, true);
            AppendConsoleLine("Project directory set to: " + model_.project_root.string());
            mark_settings_dirty();
          }
          else
          {
            AppendConsoleLine("Warning: Project directory does not exist: " + model_.settings.project_root_path);
          }
        }

        char build_dir_buffer[1024];
        std::strncpy(build_dir_buffer, model_.build_dir_input.c_str(), sizeof(build_dir_buffer) - 1);
        build_dir_buffer[sizeof(build_dir_buffer) - 1] = '\0';
        if (ImGui::InputText("Build directory", build_dir_buffer, sizeof(build_dir_buffer)))
        {
          model_.build_dir_input = Editor::Trim(std::string(build_dir_buffer));
          model_.build_dir_input = NormalizeProjectRelativePathText(model_.project_root, std::string(build_dir_buffer));
          model_.settings.build_directory = model_.build_dir_input;
          model_.settings.last_build_dir = model_.settings.build_directory;
          mark_settings_dirty();
        }

        char executable_buffer[1024];
        std::strncpy(executable_buffer, model_.executable_input.c_str(), sizeof(executable_buffer) - 1);
        executable_buffer[sizeof(executable_buffer) - 1] = '\0';
        if (ImGui::InputText("Executable path", executable_buffer, sizeof(executable_buffer)))
        {
          model_.executable_input = executable_buffer;
          model_.settings.last_executable_path = model_.executable_input;
          mark_settings_dirty();
        }

        char export_directory_buffer[1024];
        std::strncpy(export_directory_buffer, model_.settings.export_directory.c_str(), sizeof(export_directory_buffer) - 1);
        export_directory_buffer[sizeof(export_directory_buffer) - 1] = '\0';
        if (ImGui::InputText("Export directory", export_directory_buffer, sizeof(export_directory_buffer)))
        {
          model_.settings.export_directory = Editor::Trim(std::string(export_directory_buffer));
          mark_settings_dirty();
        }
        ImGui::SameLine();
        if (ImGui::Button("Browse export"))
        {
          model_.export_browser_relative_path = DisplayPathToMountRelative(model_.project_mount_source_path, model_.settings.export_directory, model_.settings.project_mount_path);
          model_.show_export_directory_dialog = true;
          model_.show_open_project_dialog = false;
          model_.show_new_project_dialog = false;
          RefreshExportDirectoryBrowser(model_.export_browser_relative_path);
        }

        char header_dirs_buffer[2048];
        std::strncpy(header_dirs_buffer, model_.settings.header_search_dirs.c_str(), sizeof(header_dirs_buffer) - 1);
        header_dirs_buffer[sizeof(header_dirs_buffer) - 1] = '\0';
        if (ImGui::InputTextMultiline("Header directories", header_dirs_buffer, sizeof(header_dirs_buffer), ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 3.0f)))
        {
          model_.settings.header_search_dirs = Editor::Trim(std::string(header_dirs_buffer));
          mark_settings_dirty();
        }

        char library_dirs_buffer[2048];
        std::strncpy(library_dirs_buffer, model_.settings.library_search_dirs.c_str(), sizeof(library_dirs_buffer) - 1);
        library_dirs_buffer[sizeof(library_dirs_buffer) - 1] = '\0';
        if (ImGui::InputTextMultiline("Library directories", library_dirs_buffer, sizeof(library_dirs_buffer), ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 3.0f)))
        {
          model_.settings.library_search_dirs = Editor::Trim(std::string(library_dirs_buffer));
          mark_settings_dirty();
        }
        if (ImGui::Button("Refresh paths"))
        {
          const std::string previous_library_default = model_.default_library_search_dirs;
          RefreshBuildOptions();
          RefreshPaths(true);
          ApplySearchDirDefaults(model_.settings, model_.default_header_search_dirs, model_.default_library_search_dirs, previous_library_default);
          model_.build_dir_input = NormalizeProjectRelativePathText(model_.project_root, model_.build_dir_input);
          model_.settings.build_directory = model_.build_dir_input;
          model_.settings.last_build_dir = model_.settings.build_directory;
          model_.settings.last_executable_path = model_.executable_input;
          mark_settings_dirty();
        }

        ImGui::Separator();
        ImGui::BeginDisabled(model_.bridge_build_in_progress);
        const std::string build_button_label = "Build " + (model_.settings.target_executable_name.empty() ? std::string{"TestECS"} : model_.settings.target_executable_name);
        if (ImGui::Button(build_button_label.c_str()))
        {
          model_.run_after_build_request = false;
          BuildTestECS();
        }
        ImGui::SameLine();
        if (ImGui::Button("Build && Run"))
        {
          model_.run_after_build_request = true;
          BuildTestECS();
          if (!Editor::ShouldUseSocketBuilds(model_.settings) && model_.build_succeeded)
          {
            model_.run_after_build_request = false;
            RunTestECS();
          }
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop"))
        {
          StopGame();
        }
        ImGui::EndDisabled();
        if (model_.bridge_build_in_progress)
        {
          ImGui::TextDisabled("A bridge build is already running.");
        }
        if (model_.bridge_rebuild_needed)
        {
          ImGui::TextColored(ImVec4(0.95f, 0.75f, 0.25f, 1.0f), "Rebuild needed: %d changed file(s) detected.", model_.bridge_changed_file_count);
        }

        ImGui::Separator();
        ImGui::TextWrapped("Status: %s", model_.status.c_str());
        ImGui::Text("Project root: %s", model_.project_root.string().c_str());
        ImGui::Text("Build dir: %s", model_.build_dir.empty() ? "<unset>" : model_.build_dir.string().c_str());
        ImGui::Text("Executable: %s", model_.game_executable.empty() ? "not found" : model_.game_executable.string().c_str());
        ImGui::Text("Game process ID: %lld", static_cast<long long>(model_.game_pid));
        if (model_.build_bridge_warning_active)
        {
          ImGui::TextColored(ImVec4(0.95f, 0.45f, 0.30f, 1.0f), "Bridge warning: connection probes are failing.");
        }

        const ImVec2 build_workflow_size = ImGui::GetWindowSize();
        const int build_workflow_width = std::max(1, static_cast<int>(build_workflow_size.x));
        const int build_workflow_height = std::max(1, static_cast<int>(build_workflow_size.y));
        if (model_.settings.panel_build_workflow_width != build_workflow_width || model_.settings.panel_build_workflow_height != build_workflow_height)
        {
          model_.settings.panel_build_workflow_width = build_workflow_width;
          model_.settings.panel_build_workflow_height = build_workflow_height;
          mark_settings_dirty();
        }

        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Settings"))
      {
        ImGui::Text("Theme");
        const int safe_theme_index = std::max(0, std::min(model_.selected_theme_preset_index, static_cast<int>(model_.theme_presets.size()) - 1));
        const char *current_theme_label = model_.theme_presets.empty() ? "<none>" : model_.theme_presets[safe_theme_index].name.c_str();
        if (ImGui::BeginCombo("##ProjectThemePreset", current_theme_label))
        {
          for (int i = 0; i < static_cast<int>(model_.theme_presets.size()); ++i)
          {
            const bool selected = i == model_.selected_theme_preset_index;
            if (ImGui::Selectable(model_.theme_presets[i].name.c_str(), selected))
            {
              ApplyThemePresetByIndex(i);
              ApplyThemeCustomizations();
              PersistThemePreference();
            }
            if (selected)
            {
              ImGui::SetItemDefaultFocus();
            }
          }
          ImGui::EndCombo();
        }

        int font_scale_percent = static_cast<int>(model_.ui_font_scale * 100.0f + 0.5f);
        if (ImGui::SliderInt("Font scale (%)", &font_scale_percent, 80, 180))
        {
          model_.ui_font_scale = static_cast<float>(font_scale_percent) / 100.0f;
          ApplyThemeCustomizations();
          PersistThemePreference();
        }

        int spacing_percent = static_cast<int>(model_.ui_spacing_density * 100.0f + 0.5f);
        if (ImGui::SliderInt("Spacing density (%)", &spacing_percent, 70, 150))
        {
          model_.ui_spacing_density = static_cast<float>(spacing_percent) / 100.0f;
          ApplyThemeCustomizations();
          PersistThemePreference();
        }

        int rounding_value = static_cast<int>(model_.ui_rounding + 0.5f);
        if (ImGui::SliderInt("Corner rounding", &rounding_value, 0, 16))
        {
          model_.ui_rounding = static_cast<float>(rounding_value);
          ApplyThemeCustomizations();
          PersistThemePreference();
        }

        float accent_rgb[3] = {model_.ui_accent_color.x, model_.ui_accent_color.y, model_.ui_accent_color.z};
        if (ImGui::ColorEdit3("Accent color", accent_rgb))
        {
          model_.ui_accent_color.x = accent_rgb[0];
          model_.ui_accent_color.y = accent_rgb[1];
          model_.ui_accent_color.z = accent_rgb[2];
          model_.ui_accent_color.w = 1.0f;
          ApplyThemeCustomizations();
          PersistThemePreference();
        }

        ImGui::Separator();
        ImGui::Text("Layout");
#ifdef IMGUI_HAS_DOCK
        if (model_.layout_presets.empty())
        {
          model_.layout_presets = DefaultLayoutPresets();
        }

        const int safe_layout_index = std::max(0, std::min(model_.selected_layout_preset_index, static_cast<int>(model_.layout_presets.size()) - 1));
        const char *current_layout_label = model_.layout_presets.empty() ? "<none>" : model_.layout_presets[safe_layout_index].name.c_str();
        if (ImGui::BeginCombo("##ProjectDockLayoutPreset", current_layout_label))
        {
          for (int i = 0; i < static_cast<int>(model_.layout_presets.size()); ++i)
          {
            const bool selected = i == model_.selected_layout_preset_index;
            if (ImGui::Selectable(model_.layout_presets[i].name.c_str(), selected))
            {
              model_.selected_layout_preset_index = i;
              model_.dock_layout_apply_requested = true;
              model_.settings.layout_preset_index = model_.selected_layout_preset_index;
              mark_settings_dirty();
            }
            if (selected)
            {
              ImGui::SetItemDefaultFocus();
            }
          }
          ImGui::EndCombo();
        }
        ImGui::SameLine();
        if (ImGui::Button("Apply layout"))
        {
          model_.dock_layout_apply_requested = true;
          model_.settings.layout_preset_index = model_.selected_layout_preset_index;
          mark_settings_dirty();
        }
#else
        ImGui::TextDisabled("Docking not enabled in this build.");
#endif

        ImGui::EndTabItem();
      }

      ImGui::EndTabBar();
    }

    ImGui::Separator();
    ImGui::BeginDisabled(!model_.project_settings_dirty);
    if (ImGui::Button("Save"))
    {
      model_.settings.panel_build_workflow_open = model_.show_project_window;
      model_.settings.panel_scene_open = model_.show_scene_window;
      model_.settings.panel_preview_open = model_.show_preview_window;
      model_.settings.panel_console_open = model_.show_console_window;
      model_.settings.panel_game_log_open = model_.show_game_log_window;
      model_.settings.compiler_toolkit = model_.compiler_presets[std::max(0, std::min(model_.selected_preset_index, static_cast<int>(model_.compiler_presets.size()) - 1))].name;
      model_.settings.last_compiler_preset = model_.settings.compiler_toolkit;
      model_.build_dir_input = NormalizeProjectRelativePathText(model_.project_root, model_.build_dir_input);
      model_.settings.build_directory = model_.build_dir_input;
      model_.settings.last_build_dir = model_.settings.build_directory;
      model_.settings.last_executable_path = model_.executable_input;
      if (model_.settings.build_profiles.empty())
      {
        model_.settings.selected_build_profile = model_.settings.selected_build_profile.empty() ? std::string{"Default"} : model_.settings.selected_build_profile;
        model_.settings.build_profiles.push_back(CaptureBuildProfileFromSettings(model_.settings, model_.executable_input));
        model_.selected_build_profile_index = 0;
      }
      model_.selected_build_profile_index = std::max(0, std::min(model_.selected_build_profile_index, static_cast<int>(model_.settings.build_profiles.size()) - 1));
      model_.settings.selected_build_profile = model_.settings.build_profiles[model_.selected_build_profile_index].name;
      model_.settings.build_profiles[model_.selected_build_profile_index] = CaptureBuildProfileFromSettings(model_.settings, model_.executable_input);
      model_.settings.layout_preset_index = model_.selected_layout_preset_index;
      std::filesystem::path settings_project_root = model_.project_root;
      const std::filesystem::path configured_settings_root(model_.settings.project_root_path);
      if (!configured_settings_root.empty())
      {
        std::error_code root_error;
        if (std::filesystem::exists(configured_settings_root, root_error) && std::filesystem::is_directory(configured_settings_root, root_error))
        {
          settings_project_root = std::filesystem::absolute(configured_settings_root);
        }
      }

      bool settings_saved = false;
      std::string settings_save_path;
      if (Editor::ShouldUseSocketBuilds(model_.settings) && !model_.settings.project_mount_path.empty())
      {
        std::string response;
        std::string bridge_error;
        const std::string request = Editor::CreateProjectSettingsSaveRequest(model_.settings);
        if (Editor::SendBuildBridgeRequestWithFallback(model_.settings, request, response, bridge_error, nullptr, 1000, 3000))
        {
          settings_saved = Editor::ExtractJsonBool(response, "success", false);
          settings_save_path = Editor::ExtractJsonString(response, "settingsPath");
          if (!settings_saved)
          {
            const std::string bridge_status = Editor::ExtractJsonString(response, "status");
            AppendConsoleLine(bridge_status.empty() ? "Warning: Build bridge failed to persist project settings." : bridge_status);
          }
        }
        else
        {
          AppendConsoleLine("Warning: Project settings bridge save failed: " + bridge_error);
        }
      }
      else
      {
        const std::filesystem::path settings_write_path = Editor::GetEditorSettingsWritePath(settings_project_root);
        settings_save_path = settings_write_path.string();
        settings_saved = Editor::SaveEditorSettings(settings_project_root, model_.settings);
      }

      if (settings_saved)
      {
        model_.project_root = settings_project_root;
        model_.project_settings_dirty = false;
        model_.status = "Project settings saved.";
        AppendConsoleLine("Project settings saved: " + (settings_save_path.empty() ? std::string{"<bridge>"} : settings_save_path));
      }
      else
      {
        AppendConsoleLine("Warning: Failed to persist project settings" + (settings_save_path.empty() ? std::string{"."} : std::string{": "} + settings_save_path));
      }
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::TextDisabled("%s", model_.project_settings_dirty ? "Unsaved changes" : "Saved");

    ImGui::End();
  }

  if (model_.show_preview_window)
  {
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(model_.settings.panel_preview_width), static_cast<float>(model_.settings.panel_preview_height)), ImGuiCond_FirstUseEver);
    ImGui::Begin("Preview", &model_.show_preview_window);
    const float control_block_width = 280.0f;
    const float available_width = ImGui::GetContentRegionAvail().x;
    if (available_width > control_block_width)
    {
      ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (available_width - control_block_width) * 0.5f);
    }
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 4.0f));
    if (ImGui::Button("Play"))
    {
      StartPreviewGame();
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(model_.bridge_build_in_progress);
    if (ImGui::Button("Rebuild"))
    {
      model_.run_after_build_request = false;
      BuildTestECS();
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button("Stop"))
    {
      StopPreviewGame();
    }
    ImGui::PopStyleVar();
    ImGui::Separator();

    if (model_.preview_game && model_.preview_game->IsEngineRunning())
    {
      const ImVec2 available = ImGui::GetContentRegionAvail();
      const float content_w = std::max(320.0f, available.x - 8.0f);
      const float content_h = std::max(240.0f, available.y - 8.0f);
      const float preview_aspect = 800.0f / 600.0f;

      float display_w = content_w;
      float display_h = display_w / preview_aspect;
      if (display_h > content_h)
      {
        display_h = content_h;
        display_w = display_h * preview_aspect;
      }

      const int tex_w = std::max(320, static_cast<int>(display_w));
      const int tex_h = std::max(240, static_cast<int>(display_h));

      if (!model_.preview_texture_ready || model_.preview_texture.texture.width != tex_w || model_.preview_texture.texture.height != tex_h)
      {
        model_.preview_texture = LoadRenderTexture(tex_w, tex_h);
        model_.preview_texture_ready = true;
      }

      const double now = GetTime();
      const double preview_tick_interval = 1.0 / 60.0;
      if (model_.preview_last_tick_time <= 0.0 || now - model_.preview_last_tick_time >= preview_tick_interval)
      {
        ScopedCurrentPath preview_cwd(model_.project_root);
        model_.preview_game->TickHostedFrame();
        model_.preview_last_tick_time = now;
      }

      BeginTextureMode(model_.preview_texture);
      ClearBackground(BLACK);
      {
        ScopedCurrentPath preview_cwd(model_.project_root);
        model_.preview_game->RenderHostedFrame();
      }
      EndTextureMode();

      const float offset_x = std::max(0.0f, (content_w - display_w) * 0.5f);
      ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset_x);
      ImGui::Image((ImTextureID)(intptr_t)model_.preview_texture.texture.id, ImVec2(display_w, display_h), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

      if (ImGui::GetCurrentContext() != nullptr)
      {
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        const ImVec2 overlay_pos = ImVec2(ImGui::GetCursorScreenPos().x + 10.0f, ImGui::GetCursorScreenPos().y + 10.0f);
        draw_list->AddRectFilled(overlay_pos, ImVec2(overlay_pos.x + 220.0f, overlay_pos.y + 46.0f), IM_COL32(0, 0, 0, 160));
        draw_list->AddText(ImVec2(overlay_pos.x + 8.0f, overlay_pos.y + 6.0f), IM_COL32(255, 255, 255, 255), "Preview running");
        draw_list->AddText(ImVec2(overlay_pos.x + 8.0f, overlay_pos.y + 24.0f), IM_COL32(180, 220, 255, 255), "60 FPS target");
      }
    }
    else
    {
      ImGui::TextWrapped("Use Play above to start the hosted preview in this panel.");
    }

    const ImVec2 preview_size = ImGui::GetWindowSize();
    const int preview_width = std::max(1, static_cast<int>(preview_size.x));
    const int preview_height = std::max(1, static_cast<int>(preview_size.y));
    if (model_.settings.panel_preview_width != preview_width || model_.settings.panel_preview_height != preview_height)
    {
      model_.settings.panel_preview_width = preview_width;
      model_.settings.panel_preview_height = preview_height;
      mark_settings_dirty();
    }

    ImGui::End();
  }

  if (model_.show_scene_window)
  {
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(model_.settings.panel_scene_width), static_cast<float>(model_.settings.panel_scene_height)), ImGuiCond_FirstUseEver);
    ImGui::Begin("Scene", &model_.show_scene_window);
    ImGui::TextWrapped("Scene view placeholder. This panel is intended for object placement, transform gizmos, and component editing workflows.");
    ImGui::Separator();
    ImGui::BeginChild("SceneViewportPlaceholder", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_NoScrollbar);
    ImGui::TextDisabled("Scene viewport rendering target will appear here.");
    ImGui::EndChild();

    const ImVec2 scene_size = ImGui::GetWindowSize();
    const int scene_width = std::max(1, static_cast<int>(scene_size.x));
    const int scene_height = std::max(1, static_cast<int>(scene_size.y));
    if (model_.settings.panel_scene_width != scene_width || model_.settings.panel_scene_height != scene_height)
    {
      model_.settings.panel_scene_width = scene_width;
      model_.settings.panel_scene_height = scene_height;
      mark_settings_dirty();
    }

    ImGui::End();
  }

  if (model_.show_console_window)
  {
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(model_.settings.panel_console_width), static_cast<float>(model_.settings.panel_console_height)), ImGuiCond_FirstUseEver);
    ImGui::Begin("Console", &model_.show_console_window);
    if (ImGui::Button("Clear"))
    {
      model_.console_lines.clear();
    }
    ImGui::SameLine();
    ImGui::Text("%zu lines", model_.console_lines.size());

    ImGui::BeginChild("ConsoleScroll", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto &line : model_.console_lines)
    {
      RenderAnsiTextLine(line);
    }
    if (model_.console_scroll_to_bottom)
    {
      ImGui::SetScrollHereY(1.0f);
      model_.console_scroll_to_bottom = false;
    }
    ImGui::EndChild();

    const ImVec2 console_size = ImGui::GetWindowSize();
    const int console_width = std::max(1, static_cast<int>(console_size.x));
    const int console_height = std::max(1, static_cast<int>(console_size.y));
    if (model_.settings.panel_console_width != console_width || model_.settings.panel_console_height != console_height)
    {
      model_.settings.panel_console_width = console_width;
      model_.settings.panel_console_height = console_height;
      mark_settings_dirty();
    }

    ImGui::End();
  }

  if (model_.show_game_log_window)
  {
    ImGui::SetNextWindowSize(ImVec2(720.0f, 240.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("Game Log", &model_.show_game_log_window);
    if (ImGui::Button("Clear"))
    {
      model_.game_log_lines.clear();
    }
    ImGui::SameLine();
    ImGui::Text("%zu lines", model_.game_log_lines.size());

    ImGui::BeginChild("GameLogScroll", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto &line : model_.game_log_lines)
    {
      RenderAnsiTextLine(line);
    }
    if (model_.game_log_scroll_to_bottom)
    {
      ImGui::SetScrollHereY(1.0f);
      model_.game_log_scroll_to_bottom = false;
    }
    ImGui::EndChild();
    ImGui::End();
  }

  if (model_.show_open_project_dialog)
  {
    ImGui::SetNextWindowSize(ImVec2(560.0f, 420.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("Open Project", &model_.show_open_project_dialog);
    ImGui::Text("Mount: %s", model_.project_browser_mount_path.empty() ? "<unknown>" : model_.project_browser_mount_path.c_str());
    ImGui::Text("Path: /%s", model_.project_browser_relative_path.c_str());
    if (ImGui::Button("Refresh"))
    {
      RefreshProjectBrowser(model_.project_browser_relative_path);
    }
    ImGui::SameLine();
    if (ImGui::Button("Up"))
    {
      const std::filesystem::path current(model_.project_browser_relative_path);
      const std::filesystem::path parent = current.parent_path();
      RefreshProjectBrowser(parent == "." ? std::string{} : parent.generic_string());
    }
    ImGui::SameLine();
    if (ImGui::Button("Open Current"))
    {
      const std::filesystem::path previous_root = model_.project_root;
      OpenBridgeProject(model_.project_browser_relative_path);
      if (model_.project_root != previous_root)
      {
        mark_settings_dirty();
        model_.show_open_project_dialog = false;
      }
    }
    if (!model_.project_browser_status.empty())
    {
      ImGui::TextWrapped("%s", model_.project_browser_status.c_str());
    }
    ImGui::Separator();
    ImGui::BeginChild("OpenProjectBrowser", ImVec2(0.0f, 0.0f), true);
    for (const auto &directory : model_.project_browser_directories)
    {
      if (ImGui::Selectable(directory.c_str()))
      {
        const std::filesystem::path child_path = std::filesystem::path(model_.project_browser_relative_path) / directory;
        RefreshProjectBrowser(child_path.generic_string());
      }
    }
    ImGui::EndChild();
    ImGui::End();
  }

  if (model_.show_new_project_dialog)
  {
    ImGui::SetNextWindowSize(ImVec2(560.0f, 460.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("New Project", &model_.show_new_project_dialog);
    ImGui::Text("Mount: %s", model_.project_browser_mount_path.empty() ? "<unknown>" : model_.project_browser_mount_path.c_str());
    ImGui::Text("Parent: /%s", model_.project_browser_relative_path.c_str());
    char new_project_name_buffer[256];
    std::strncpy(new_project_name_buffer, model_.new_project_name.c_str(), sizeof(new_project_name_buffer) - 1);
    new_project_name_buffer[sizeof(new_project_name_buffer) - 1] = '\0';
    if (ImGui::InputText("Project name", new_project_name_buffer, sizeof(new_project_name_buffer)))
    {
      model_.new_project_name = Editor::Trim(std::string(new_project_name_buffer));
    }
    if (ImGui::Button("Refresh"))
    {
      RefreshProjectBrowser(model_.project_browser_relative_path);
    }
    ImGui::SameLine();
    if (ImGui::Button("Up"))
    {
      const std::filesystem::path current(model_.project_browser_relative_path);
      const std::filesystem::path parent = current.parent_path();
      RefreshProjectBrowser(parent == "." ? std::string{} : parent.generic_string());
    }
    ImGui::SameLine();
    if (ImGui::Button("Create Here"))
    {
      const std::filesystem::path previous_root = model_.project_root;
      CreateBridgeProject(model_.project_browser_relative_path, model_.new_project_name);
      if (model_.project_root != previous_root)
      {
        mark_settings_dirty();
        model_.show_new_project_dialog = false;
      }
    }
    if (!model_.project_browser_status.empty())
    {
      ImGui::TextWrapped("%s", model_.project_browser_status.c_str());
    }
    ImGui::Separator();
    ImGui::BeginChild("NewProjectBrowser", ImVec2(0.0f, 0.0f), true);
    for (const auto &directory : model_.project_browser_directories)
    {
      if (ImGui::Selectable(directory.c_str()))
      {
        const std::filesystem::path child_path = std::filesystem::path(model_.project_browser_relative_path) / directory;
        RefreshProjectBrowser(child_path.generic_string());
      }
    }
    ImGui::EndChild();
    ImGui::End();
  }

  if (model_.show_export_directory_dialog)
  {
    ImGui::SetNextWindowSize(ImVec2(560.0f, 460.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("Export Directory", &model_.show_export_directory_dialog);
    ImGui::Text("Mount: %s", model_.project_mount_source_path.empty() ? "<unknown>" : model_.project_mount_source_path.c_str());
    ImGui::Text("Path: /%s", model_.export_browser_relative_path.c_str());
    char new_directory_name_buffer[256];
    std::strncpy(new_directory_name_buffer, model_.new_export_directory_name.c_str(), sizeof(new_directory_name_buffer) - 1);
    new_directory_name_buffer[sizeof(new_directory_name_buffer) - 1] = '\0';
    if (ImGui::InputText("New directory", new_directory_name_buffer, sizeof(new_directory_name_buffer)))
    {
      model_.new_export_directory_name = Editor::Trim(std::string(new_directory_name_buffer));
    }
    if (ImGui::Button("Refresh"))
    {
      RefreshExportDirectoryBrowser(model_.export_browser_relative_path);
    }
    ImGui::SameLine();
    if (ImGui::Button("Up"))
    {
      const std::filesystem::path current(model_.export_browser_relative_path);
      const std::filesystem::path parent = current.parent_path();
      RefreshExportDirectoryBrowser(parent == "." ? std::string{} : parent.generic_string());
    }
    ImGui::SameLine();
    if (ImGui::Button("Use Current"))
    {
      model_.settings.export_directory = JoinSourceMountPath(model_.project_mount_source_path, model_.export_browser_relative_path);
      mark_settings_dirty();
      model_.show_export_directory_dialog = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Create Here"))
    {
      std::string created_relative_path;
      if (CreateBridgeDirectory(model_.export_browser_relative_path, model_.new_export_directory_name, created_relative_path))
      {
        model_.settings.export_directory = JoinSourceMountPath(model_.project_mount_source_path, created_relative_path);
        model_.export_browser_relative_path = created_relative_path;
        model_.new_export_directory_name.clear();
        mark_settings_dirty();
        RefreshExportDirectoryBrowser(model_.export_browser_relative_path);
      }
    }
    if (!model_.export_browser_status.empty())
    {
      ImGui::TextWrapped("%s", model_.export_browser_status.c_str());
    }
    ImGui::Separator();
    ImGui::BeginChild("ExportDirectoryBrowser", ImVec2(0.0f, 0.0f), true);
    for (const auto &directory : model_.export_browser_directories)
    {
      if (ImGui::Selectable(directory.c_str()))
      {
        const std::filesystem::path child_path = std::filesystem::path(model_.export_browser_relative_path) / directory;
        RefreshExportDirectoryBrowser(child_path.generic_string());
      }
    }
    ImGui::EndChild();
    ImGui::End();
  }

  if (model_.show_source_directory_dialog)
  {
    ImGui::SetNextWindowSize(ImVec2(520.0f, 420.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("Source Directory", &model_.show_source_directory_dialog);
    ImGui::Text("Project root: %s", model_.project_root.empty() ? "<unknown>" : model_.project_root.string().c_str());
    ImGui::Text("Path: /%s", model_.source_browser_relative_path.c_str());
    if (ImGui::Button("Refresh"))
    {
      RefreshSourceDirectoryBrowser(model_.source_browser_relative_path);
    }
    ImGui::SameLine();
    if (ImGui::Button("Up"))
    {
      const std::filesystem::path current(model_.source_browser_relative_path);
      const std::filesystem::path parent = current.parent_path();
      RefreshSourceDirectoryBrowser(parent == "." ? std::string{} : parent.generic_string());
    }
    ImGui::SameLine();
    if (ImGui::Button("Use Current"))
    {
      model_.settings.source_directory = model_.source_browser_relative_path;
      mark_settings_dirty();
      model_.show_source_directory_dialog = false;
    }
    if (!model_.source_browser_status.empty())
    {
      ImGui::TextWrapped("%s", model_.source_browser_status.c_str());
    }
    ImGui::Separator();
    ImGui::BeginChild("SourceDirectoryBrowser", ImVec2(0.0f, 0.0f), true);
    for (const auto &directory : model_.source_browser_directories)
    {
      if (ImGui::Selectable(directory.c_str()))
      {
        const std::filesystem::path child_path = std::filesystem::path(model_.source_browser_relative_path) / directory;
        RefreshSourceDirectoryBrowser(child_path.generic_string());
      }
    }
    ImGui::EndChild();
    ImGui::End();
  }

  if (model_.show_resources_directory_dialog)
  {
    ImGui::SetNextWindowSize(ImVec2(520.0f, 420.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("Resources Directory", &model_.show_resources_directory_dialog);
    ImGui::Text("Project root: %s", model_.project_root.empty() ? "<unknown>" : model_.project_root.string().c_str());
    ImGui::Text("Path: /%s", model_.resources_browser_relative_path.c_str());
    if (ImGui::Button("Refresh"))
    {
      RefreshResourcesDirectoryBrowser(model_.resources_browser_relative_path);
    }
    ImGui::SameLine();
    if (ImGui::Button("Up"))
    {
      const std::filesystem::path current(model_.resources_browser_relative_path);
      const std::filesystem::path parent = current.parent_path();
      RefreshResourcesDirectoryBrowser(parent == "." ? std::string{} : parent.generic_string());
    }
    ImGui::SameLine();
    if (ImGui::Button("Use Current"))
    {
      model_.settings.resources_directory = model_.resources_browser_relative_path;
      mark_settings_dirty();
      model_.show_resources_directory_dialog = false;
    }
    if (!model_.resources_browser_status.empty())
    {
      ImGui::TextWrapped("%s", model_.resources_browser_status.c_str());
    }
    ImGui::Separator();
    ImGui::BeginChild("ResourcesDirectoryBrowser", ImVec2(0.0f, 0.0f), true);
    for (const auto &directory : model_.resources_browser_directories)
    {
      if (ImGui::Selectable(directory.c_str()))
      {
        const std::filesystem::path child_path = std::filesystem::path(model_.resources_browser_relative_path) / directory;
        RefreshResourcesDirectoryBrowser(child_path.generic_string());
      }
    }
    ImGui::EndChild();
    ImGui::End();
  }

  rlImGuiEnd();

  ImDrawData *draw_data = ImGui::GetDrawData();
  if (draw_data == nullptr || draw_data->CmdListsCount == 0)
  {
    DrawText("Myriad Editor UI produced no ImGui draw data", 24, 24, 20, RED);
    DrawText("Check editor resource/config paths in the console output.", 24, 52, 20, RED);
  }
}

void MyriadEditor::AppendConsoleLine(const std::string &line)
{
  model_.console_lines.push_back(line);
  model_.console_scroll_to_bottom = true;
}

void MyriadEditor::PreShutdown()
{
  JoinBridgeBuildThread();
  StopGame();
  StopPreviewGame();
  EndPreviewLogCapture();
  UnloadHostedPreviewLibrary();
  rlImGuiShutdown();
}

void MyriadEditor::JoinBridgeBuildThread()
{
#if MYRIAD_EDITOR_ENABLE_BRIDGE_THREADS
  if (bridge_build_thread_.joinable())
  {
    bridge_build_thread_.join();
  }
#endif
}

void MyriadEditor::RestartPreviewForLatestBuild()
{
  model_.hosted_library_reload_required = true;
  if (!model_.preview_game || !model_.preview_game->IsEngineRunning())
  {
    return;
  }

  AppendConsoleLine("Rebuild completed. Reloading hosted preview library.");
  StopPreviewGame();
  StartPreviewGame();
}

bool MyriadEditor::ReloadHostedPreviewLibrary()
{
  UnloadHostedPreviewLibrary();

  model_.hosted_library_path = FindHostedLibraryPath(model_.build_dir, model_.settings.target_executable_name);
  if (model_.hosted_library_path.empty())
  {
    AppendConsoleLine("Hosted preview library was not found in build output.");
    return false;
  }

  std::error_code fs_error;
  const std::filesystem::path cache_dir = model_.build_dir / "Editor" / ".myriad_preview_cache";
  std::filesystem::create_directories(cache_dir, fs_error);
  if (fs_error)
  {
    AppendConsoleLine("Failed to prepare preview cache directory: " + fs_error.message());
    return false;
  }

  const auto now_ticks = std::chrono::duration_cast<std::chrono::microseconds>(
                             std::chrono::system_clock::now().time_since_epoch())
                             .count();
  model_.hosted_library_loaded_copy_path = cache_dir / (model_.hosted_library_path.stem().string() + "_preview_" + std::to_string(now_ticks) + model_.hosted_library_path.extension().string());

  std::filesystem::copy_file(model_.hosted_library_path, model_.hosted_library_loaded_copy_path, std::filesystem::copy_options::overwrite_existing, fs_error);
  if (fs_error)
  {
    AppendConsoleLine("Failed to stage hosted preview library copy: " + fs_error.message());
    model_.hosted_library_loaded_copy_path.clear();
    return false;
  }

#ifdef _WIN32
  HMODULE module = LoadLibraryA(model_.hosted_library_loaded_copy_path.string().c_str());
  if (module == nullptr)
  {
    AppendConsoleLine("Failed to load hosted preview library: " + model_.hosted_library_loaded_copy_path.string());
    model_.hosted_library_loaded_copy_path.clear();
    return false;
  }

  using HostedCreateFn = Myriad::MyrGameApplication *(*)();
  HostedCreateFn create_fn = reinterpret_cast<HostedCreateFn>(GetProcAddress(module, "Myriad_CreateHostedGame"));
  model_.hosted_destroy_fn = reinterpret_cast<Editor::HostedDestroyFn>(GetProcAddress(module, "Myriad_DestroyHostedGame"));
  if (create_fn == nullptr || model_.hosted_destroy_fn == nullptr)
  {
    AppendConsoleLine("Hosted preview symbols were not found in library.");
    FreeLibrary(module);
    model_.hosted_library_loaded_copy_path.clear();
    return false;
  }

  model_.preview_game = create_fn();
  if (model_.preview_game == nullptr)
  {
    AppendConsoleLine("Hosted preview factory returned no game instance.");
    FreeLibrary(module);
    model_.hosted_destroy_fn = nullptr;
    model_.hosted_library_loaded_copy_path.clear();
    return false;
  }

  model_.hosted_library_handle = module;
#else
  void *module = dlopen(model_.hosted_library_loaded_copy_path.c_str(), RTLD_NOW | RTLD_LOCAL);
  if (module == nullptr)
  {
    AppendConsoleLine(std::string("Failed to load hosted preview library: ") + dlerror());
    model_.hosted_library_loaded_copy_path.clear();
    return false;
  }

  using HostedCreateFn = Myriad::MyrGameApplication *(*)();
  HostedCreateFn create_fn = reinterpret_cast<HostedCreateFn>(dlsym(module, "Myriad_CreateHostedGame"));
  model_.hosted_destroy_fn = reinterpret_cast<Editor::HostedDestroyFn>(dlsym(module, "Myriad_DestroyHostedGame"));
  if (create_fn == nullptr || model_.hosted_destroy_fn == nullptr)
  {
    AppendConsoleLine("Hosted preview symbols were not found in library.");
    dlclose(module);
    model_.hosted_library_loaded_copy_path.clear();
    return false;
  }

  model_.preview_game = create_fn();
  if (model_.preview_game == nullptr)
  {
    AppendConsoleLine("Hosted preview factory returned no game instance.");
    dlclose(module);
    model_.hosted_destroy_fn = nullptr;
    model_.hosted_library_loaded_copy_path.clear();
    return false;
  }

  model_.hosted_library_handle = module;
#endif

  model_.hosted_library_reload_required = false;
  AppendConsoleLine("Hosted preview library loaded: " + model_.hosted_library_path.string());
  return true;
}

void MyriadEditor::UnloadHostedPreviewLibrary()
{
  if (model_.preview_game != nullptr)
  {
    if (model_.preview_game->IsEngineRunning())
    {
      model_.preview_game->StopHosted();
    }

    if (model_.hosted_destroy_fn != nullptr)
    {
      model_.hosted_destroy_fn(model_.preview_game);
    }
    else
    {
      delete model_.preview_game;
    }
    model_.preview_game = nullptr;
  }

  model_.hosted_destroy_fn = nullptr;
  if (model_.hosted_library_handle != nullptr)
  {
#ifdef _WIN32
    FreeLibrary(static_cast<HMODULE>(model_.hosted_library_handle));
#else
    dlclose(model_.hosted_library_handle);
#endif
    model_.hosted_library_handle = nullptr;
  }

  if (!model_.hosted_library_loaded_copy_path.empty())
  {
    std::error_code fs_error;
    std::filesystem::remove(model_.hosted_library_loaded_copy_path, fs_error);
    model_.hosted_library_loaded_copy_path.clear();
  }
}

void MyriadEditor::StartBridgeBuildOverSocket(const Editor::CompilerPreset &preset, const std::string &build_dir_relative)
{
#if !MYRIAD_EDITOR_ENABLE_BRIDGE_THREADS
  (void)preset;
  (void)build_dir_relative;
  model_.build_succeeded = false;
  model_.bridge_build_in_progress = false;
  model_.status = "Socket build bridge requires std::thread support in the editor toolchain.";
  AppendConsoleLine(model_.status);
  return;
#else
  if (model_.bridge_build_in_progress)
  {
    model_.status = "A build bridge request is already in progress.";
    AppendConsoleLine(model_.status);
    return;
  }

  JoinBridgeBuildThread();

  {
    std::lock_guard<std::mutex> lock(bridge_build_mutex_);
    model_.bridge_build_pending_lines.clear();
    model_.bridge_build_result_ready = false;
    model_.bridge_build_request_success = false;
    model_.bridge_build_success = false;
    model_.bridge_build_rebuild_triggered = false;
    model_.bridge_build_response.clear();
    model_.bridge_build_error.clear();
    model_.bridge_build_status.clear();
    model_.bridge_build_executable.clear();
    model_.bridge_build_host.clear();
  }

  model_.bridge_build_in_progress = true;
  model_.status = "Building " + (model_.settings.target_executable_name.empty() ? std::string{"TestECS"} : model_.settings.target_executable_name) + " through the socket bridge...";
  model_.build_succeeded = false;
  model_.bridge_build_progress_percent = -1;

  const std::string request = Editor::CreateBuildBridgeRequest(preset,
                                                               build_dir_relative,
                                                               model_.settings.project_mount_path,
                                                               model_.settings.build_type,
                                                               model_.settings.target_executable_name,
                                                               model_.settings.source_directory,
                                                               model_.settings.resources_directory,
                                                               model_.settings.header_search_dirs,
                                                               model_.settings.library_search_dirs,
                                                               model_.settings.export_directory);
  const std::string requested_host = model_.settings.build_socket_host.empty() ? std::string{"auto"} : model_.settings.build_socket_host;
  AppendConsoleLine("Sending bridge request to " + requested_host + ":" + std::to_string(model_.settings.build_socket_port));

  Editor::EditorSettings settings_copy = model_.settings;

  bridge_build_thread_ = std::thread([this, request, settings_copy]() mutable
                                     {
    std::string response;
    std::string bridge_error;
    const bool request_success = Editor::SendBuildBridgeRequestWithFallback(
        settings_copy,
        request,
        response,
        bridge_error,
        [this](const std::string &line)
        {
          std::lock_guard<std::mutex> lock(bridge_build_mutex_);
          model_.bridge_build_pending_lines.push_back(line);
        },
        3000,
        180000);

    bool bridge_success = false;
    bool rebuild_triggered = false;
    std::string bridge_status;
    std::string bridge_executable;
    if (request_success)
    {
      bridge_success = Editor::ExtractJsonBool(response, "success", false);
      rebuild_triggered = Editor::ExtractJsonBool(response, "rebuildTriggered", true);
      bridge_status = Editor::ExtractJsonString(response, "status");
      bridge_executable = Editor::ExtractJsonString(response, "executable");
    }

    {
      std::lock_guard<std::mutex> lock(bridge_build_mutex_);
      model_.bridge_build_request_success = request_success;
      model_.bridge_build_success = bridge_success;
      model_.bridge_build_rebuild_triggered = rebuild_triggered;
      model_.bridge_build_response = response;
      model_.bridge_build_error = bridge_error;
      model_.bridge_build_status = bridge_status;
      model_.bridge_build_executable = bridge_executable;
      model_.bridge_build_host = settings_copy.build_socket_host;
      model_.bridge_build_result_ready = true;
    }

    model_.bridge_build_in_progress = false; });
#endif
}

void MyriadEditor::UpdateBridgeBuildProgressFromLine(const std::string &line)
{
  int parsed_percent = -1;
  if (!TryExtractBuildProgressPercent(line, parsed_percent))
  {
    return;
  }

  if (parsed_percent > model_.bridge_build_progress_percent)
  {
    model_.bridge_build_progress_percent = parsed_percent;
  }
}

void MyriadEditor::PumpBridgeBuildUpdates()
{
#if !MYRIAD_EDITOR_ENABLE_BRIDGE_THREADS
  return;
#else
  std::vector<std::string> pending_lines;
  bool has_result = false;
  bool request_success = false;
  bool bridge_success = false;
  bool rebuild_triggered = false;
  std::string response;
  std::string bridge_error;
  std::string bridge_status;
  std::string bridge_executable;
  std::string resolved_host;

  {
    std::lock_guard<std::mutex> lock(bridge_build_mutex_);
    pending_lines.swap(model_.bridge_build_pending_lines);
    if (model_.bridge_build_result_ready)
    {
      has_result = true;
      request_success = model_.bridge_build_request_success;
      bridge_success = model_.bridge_build_success;
      rebuild_triggered = model_.bridge_build_rebuild_triggered;
      response = model_.bridge_build_response;
      bridge_error = model_.bridge_build_error;
      bridge_status = model_.bridge_build_status;
      bridge_executable = model_.bridge_build_executable;
      resolved_host = model_.bridge_build_host;
      model_.bridge_build_result_ready = false;
    }
  }

  for (const auto &line : pending_lines)
  {
    UpdateBridgeBuildProgressFromLine(line);
    AppendConsoleLine(line);
  }

  if (!has_result)
  {
    return;
  }

  JoinBridgeBuildThread();

  if (!response.empty())
  {
    AppendConsoleLine("Bridge response: " + response);
  }

  if (!resolved_host.empty())
  {
    model_.settings.build_socket_host = resolved_host;
  }

  if (!request_success)
  {
    model_.build_succeeded = false;
    model_.run_after_build_request = false;
    model_.build_bridge_connected = false;
    model_.build_bridge_status_text = bridge_error;
    model_.status = "Build bridge error: " + bridge_error;
    AppendConsoleLine(model_.status);
    model_.bridge_build_progress_percent = -1;
    return;
  }

  model_.bridge_rebuild_needed = Editor::ExtractJsonBool(response, "rebuildNeeded", model_.bridge_rebuild_needed);
  model_.bridge_changed_file_count = std::max(0, Editor::ExtractJsonInt(response, "changedFileCount", model_.bridge_changed_file_count));
  model_.bridge_changed_files_preview = ExtractJsonStringArray(response, "changedFilesPreview", 12);

  model_.build_bridge_connected = true;
  model_.build_bridge_consecutive_failures = 0;
  model_.build_bridge_warning_active = false;
  model_.build_bridge_status_text = bridge_status.empty() ? "Bridge reachable." : bridge_status;

  if (!bridge_success)
  {
    model_.build_succeeded = false;
    model_.run_after_build_request = false;
    model_.status = bridge_status.empty() ? "The build container reported a failure." : bridge_status;
    AppendConsoleLine(model_.status);
    model_.bridge_build_progress_percent = -1;
    return;
  }

  model_.build_succeeded = true;
  model_.bridge_build_progress_percent = 100;
  model_.bridge_rebuild_needed = false;
  model_.bridge_changed_file_count = 0;
  if (!bridge_executable.empty())
  {
    model_.game_executable = Editor::ResolvePath(model_.project_root, std::filesystem::path(bridge_executable));
  }
  else
  {
    model_.game_executable = Editor::FindGameExecutable(model_.build_dir, model_.executable_input, model_.project_root, model_.settings.target_executable_name);
  }

  model_.executable_input = model_.game_executable.empty() ? model_.executable_input : model_.game_executable.string();
  model_.status = bridge_status.empty() ? "Build succeeded through the socket bridge." : bridge_status;
  AppendConsoleLine(model_.status);
  const std::string exported_executable = Editor::ExtractJsonString(response, "exportedExecutable");
  if (!exported_executable.empty())
  {
    AppendConsoleLine("Exported executable: " + exported_executable);
  }

  (void)rebuild_triggered;
  RestartPreviewForLatestBuild();

  if (model_.run_after_build_request)
  {
    model_.run_after_build_request = false;
    RunTestECS();
  }
#endif
}

void MyriadEditor::ApplyThemePresetByIndex(int preset_index)
{
  if (model_.theme_presets.empty())
  {
    model_.theme_presets = DefaultThemePresets();
  }

  if (preset_index < 0 || preset_index >= static_cast<int>(model_.theme_presets.size()))
  {
    preset_index = 0;
  }

  model_.selected_theme_preset_index = preset_index;
  const Editor::ThemePreset &preset = model_.theme_presets[preset_index];
  model_.settings.theme_preset = preset.name;

  const std::string base_lower = Editor::Lowercase(preset.base);
  if (base_lower == "light")
  {
    ImGui::StyleColorsLight();
  }
  else if (base_lower == "classic")
  {
    ImGui::StyleColorsClassic();
  }
  else
  {
    ImGui::StyleColorsDark();
  }

  ImVec4 *colors = ImGui::GetStyle().Colors;
  ImVec4 parsed_color{};
  if (ParseHexColor(preset.window_bg_hex, parsed_color))
  {
    colors[ImGuiCol_WindowBg] = parsed_color;
  }
  if (ParseHexColor(preset.header_hex, parsed_color))
  {
    colors[ImGuiCol_Header] = ImVec4(parsed_color.x, parsed_color.y, parsed_color.z, 0.86f);
  }
  if (ParseHexColor(preset.header_hovered_hex, parsed_color))
  {
    colors[ImGuiCol_HeaderHovered] = ImVec4(parsed_color.x, parsed_color.y, parsed_color.z, 0.90f);
  }
  if (ParseHexColor(preset.button_hex, parsed_color))
  {
    colors[ImGuiCol_Button] = ImVec4(parsed_color.x, parsed_color.y, parsed_color.z, 0.78f);
  }
  if (ParseHexColor(preset.button_hovered_hex, parsed_color))
  {
    colors[ImGuiCol_ButtonHovered] = ImVec4(parsed_color.x, parsed_color.y, parsed_color.z, 0.90f);
  }
  if (ParseHexColor(preset.frame_bg_hex, parsed_color))
  {
    colors[ImGuiCol_FrameBg] = ImVec4(parsed_color.x, parsed_color.y, parsed_color.z, 0.82f);
  }
  if (ParseHexColor(preset.title_bg_hex, parsed_color))
  {
    colors[ImGuiCol_TitleBg] = ImVec4(parsed_color.x, parsed_color.y, parsed_color.z, 1.0f);
  }
  if (ParseHexColor(preset.title_bg_active_hex, parsed_color))
  {
    colors[ImGuiCol_TitleBgActive] = ImVec4(parsed_color.x, parsed_color.y, parsed_color.z, 1.0f);
  }

  model_.base_theme_style = ImGui::GetStyle();
  model_.has_base_theme_style = true;
}

void MyriadEditor::ApplyThemeCustomizations()
{
  if (!model_.has_base_theme_style)
  {
    return;
  }

  ImGuiStyle tuned = model_.base_theme_style;

  auto scale_vec2 = [this](const ImVec2 &value)
  {
    return ImVec2(value.x * model_.ui_spacing_density, value.y * model_.ui_spacing_density);
  };

  tuned.WindowPadding = scale_vec2(tuned.WindowPadding);
  tuned.FramePadding = scale_vec2(tuned.FramePadding);
  tuned.ItemSpacing = scale_vec2(tuned.ItemSpacing);
  tuned.ItemInnerSpacing = scale_vec2(tuned.ItemInnerSpacing);
  tuned.CellPadding = scale_vec2(tuned.CellPadding);
  tuned.TouchExtraPadding = scale_vec2(tuned.TouchExtraPadding);

  tuned.WindowRounding = model_.ui_rounding;
  tuned.ChildRounding = model_.ui_rounding;
  tuned.PopupRounding = model_.ui_rounding;
  tuned.FrameRounding = std::max(0.0f, model_.ui_rounding * 0.65f);
  tuned.GrabRounding = std::max(0.0f, model_.ui_rounding * 0.5f);
  tuned.ScrollbarRounding = std::max(0.0f, model_.ui_rounding * 0.5f);
  tuned.TabRounding = std::max(0.0f, model_.ui_rounding * 0.5f);

  const ImVec4 accent_brighter = ShiftBrightness(model_.ui_accent_color, 0.12f);
  const ImVec4 accent_hover = ShiftBrightness(model_.ui_accent_color, 0.10f);
  const ImVec4 accent_darker = ShiftBrightness(model_.ui_accent_color, -0.08f);
  const ImVec4 accent_active = ShiftBrightness(model_.ui_accent_color, -0.05f);

  tuned.Colors[ImGuiCol_Button] = ImVec4(model_.ui_accent_color.x, model_.ui_accent_color.y, model_.ui_accent_color.z, 0.78f);
  tuned.Colors[ImGuiCol_ButtonHovered] = ImVec4(accent_brighter.x, accent_brighter.y, accent_brighter.z, 0.88f);
  tuned.Colors[ImGuiCol_ButtonActive] = ImVec4(accent_darker.x, accent_darker.y, accent_darker.z, 0.92f);
  tuned.Colors[ImGuiCol_Header] = ImVec4(model_.ui_accent_color.x, model_.ui_accent_color.y, model_.ui_accent_color.z, 0.58f);
  tuned.Colors[ImGuiCol_HeaderHovered] = ImVec4(accent_hover.x, accent_hover.y, accent_hover.z, 0.72f);
  tuned.Colors[ImGuiCol_HeaderActive] = ImVec4(accent_active.x, accent_active.y, accent_active.z, 0.86f);
  tuned.Colors[ImGuiCol_CheckMark] = model_.ui_accent_color;
  tuned.Colors[ImGuiCol_SliderGrab] = model_.ui_accent_color;
  tuned.Colors[ImGuiCol_SliderGrabActive] = accent_brighter;
  tuned.Colors[ImGuiCol_TabActive] = ImVec4(model_.ui_accent_color.x, model_.ui_accent_color.y, model_.ui_accent_color.z, 0.82f);
  tuned.Colors[ImGuiCol_TabHovered] = ImVec4(accent_brighter.x, accent_brighter.y, accent_brighter.z, 0.90f);

  ImGui::GetStyle() = tuned;
  ImGui::GetIO().FontGlobalScale = model_.ui_font_scale;
}

void MyriadEditor::PersistThemePreference()
{
  model_.settings.ui_font_scale_percent = static_cast<int>(model_.ui_font_scale * 100.0f + 0.5f);
  model_.settings.ui_rounding = static_cast<int>(model_.ui_rounding + 0.5f);
  model_.settings.ui_spacing_percent = static_cast<int>(model_.ui_spacing_density * 100.0f + 0.5f);
  model_.settings.ui_accent_hex = ToHexColor(model_.ui_accent_color);
  model_.project_settings_dirty = true;
}

void MyriadEditor::StartPreviewGame()
{
  if (model_.preview_game && model_.preview_game->IsEngineRunning())
  {
    return;
  }

  StopPreviewGame();

  // Always reload on Play so hosted preview picks up any externally rebuilt library.
  if (!ReloadHostedPreviewLibrary())
  {
    model_.status = "Failed to reload hosted preview library.";
    return;
  }

  BeginPreviewLogCapture();

  Myriad::GameEngineConfig config{};
  config.framerate = 60;
  config.window_config.resizable = true;
  config.window_config.resolution = {800, 600};
  config.window_config.fullscreen = false;
  config.window_config.vsync = false;
  strncpy(config.resource_base_path, "shared/res", sizeof(config.resource_base_path) - 1);
  const std::string resources_directory = Editor::Trim(model_.settings.resources_directory);
  if (!resources_directory.empty())
  {
    strncpy(config.resource_base_path, resources_directory.c_str(), sizeof(config.resource_base_path) - 1);
  }
  config.resource_base_path[sizeof(config.resource_base_path) - 1] = '\0';
  strncpy(config.window_title, "Embedded Preview", sizeof(config.window_title) - 1);

  bool started = false;
  if (model_.preview_game != nullptr)
  {
    ScopedCurrentPath preview_cwd(model_.project_root);
    started = model_.preview_game->StartHosted(config, false);
  }

  if (started)
  {
    model_.preview_texture = LoadRenderTexture(800, 600);
    model_.preview_texture_ready = true;
    model_.status = "Embedded preview running.";
  }
  else
  {
    EndPreviewLogCapture();
    UnloadHostedPreviewLibrary();
    model_.hosted_library_reload_required = true;
    model_.status = "Failed to start embedded preview.";
  }
}

void MyriadEditor::StopPreviewGame()
{
  if (model_.preview_game)
  {
    if (model_.preview_game->IsEngineRunning())
    {
      model_.preview_game->StopHosted();
    }
  }

  model_.preview_texture = {};
  model_.preview_texture_ready = false;
  model_.preview_stop_requested = false;
  model_.preview_texture_cleanup_requested = false;
  model_.preview_last_tick_time = 0.0;
  EndPreviewLogCapture();
}

void MyriadEditor::PumpGameLogOutput()
{
#ifndef _WIN32
  if (model_.preview_log_pipe_read_fd < 0)
  {
    // Runtime logs are still tailed below when an external game process is running.
  }
  else
  {
    char buffer[1024];
    while (true)
    {
      const ssize_t bytes_read = read(model_.preview_log_pipe_read_fd, buffer, sizeof(buffer));
      if (bytes_read <= 0)
      {
        break;
      }

      model_.preview_log_partial_line.append(buffer, static_cast<std::size_t>(bytes_read));
      std::size_t line_end = std::string::npos;
      while ((line_end = model_.preview_log_partial_line.find('\n')) != std::string::npos)
      {
        std::string line = model_.preview_log_partial_line.substr(0, line_end);
        if (!line.empty() && line.back() == '\r')
        {
          line.pop_back();
        }
        model_.game_log_lines.push_back(line);
        model_.game_log_scroll_to_bottom = true;
        model_.preview_log_partial_line.erase(0, line_end + 1);
      }
    }
  }
#endif

  if (model_.runtime_log_path.empty())
  {
    return;
  }

  std::error_code file_size_error;
  const std::uintmax_t log_size = std::filesystem::file_size(model_.runtime_log_path, file_size_error);
  if (file_size_error)
  {
    return;
  }
  if (log_size < model_.runtime_log_offset)
  {
    model_.runtime_log_offset = 0;
  }
  if (log_size == model_.runtime_log_offset)
  {
    return;
  }

  std::ifstream runtime_log(model_.runtime_log_path, std::ios::binary);
  if (!runtime_log)
  {
    return;
  }
  runtime_log.seekg(static_cast<std::streamoff>(model_.runtime_log_offset));
  std::string line;
  while (std::getline(runtime_log, line))
  {
    if (!line.empty() && line.back() == '\r')
    {
      line.pop_back();
    }
    model_.game_log_lines.push_back(line);
    model_.game_log_scroll_to_bottom = true;
  }
  model_.runtime_log_offset = log_size;
}

void MyriadEditor::BeginPreviewLogCapture()
{
#ifdef _WIN32
  return;
#else
  if (model_.preview_log_pipe_read_fd >= 0)
  {
    return;
  }

  int pipe_fds[2] = {-1, -1};
  if (pipe(pipe_fds) != 0)
  {
    model_.game_log_lines.push_back("[editor] failed to initialize preview log capture pipe");
    model_.game_log_scroll_to_bottom = true;
    return;
  }

  model_.preview_log_saved_stdout_fd = dup(STDOUT_FILENO);
  model_.preview_log_saved_stderr_fd = dup(STDERR_FILENO);
  if (model_.preview_log_saved_stdout_fd < 0 || model_.preview_log_saved_stderr_fd < 0)
  {
    close(pipe_fds[0]);
    close(pipe_fds[1]);
    model_.preview_log_saved_stdout_fd = -1;
    model_.preview_log_saved_stderr_fd = -1;
    model_.game_log_lines.push_back("[editor] failed to duplicate stdout/stderr for preview capture");
    model_.game_log_scroll_to_bottom = true;
    return;
  }

  if (dup2(pipe_fds[1], STDOUT_FILENO) < 0 || dup2(pipe_fds[1], STDERR_FILENO) < 0)
  {
    close(pipe_fds[0]);
    close(pipe_fds[1]);
    close(model_.preview_log_saved_stdout_fd);
    close(model_.preview_log_saved_stderr_fd);
    model_.preview_log_saved_stdout_fd = -1;
    model_.preview_log_saved_stderr_fd = -1;
    model_.game_log_lines.push_back("[editor] failed to redirect stdout/stderr for preview capture");
    model_.game_log_scroll_to_bottom = true;
    return;
  }

  const int flags = fcntl(pipe_fds[0], F_GETFL, 0);
  if (flags >= 0)
  {
    fcntl(pipe_fds[0], F_SETFL, flags | O_NONBLOCK);
  }

  model_.preview_log_pipe_read_fd = pipe_fds[0];
  model_.preview_log_pipe_write_fd = pipe_fds[1];
  model_.preview_log_partial_line.clear();
  model_.game_log_lines.push_back("[editor] preview log capture started");
  model_.game_log_scroll_to_bottom = true;
#endif
}

void MyriadEditor::EndPreviewLogCapture()
{
#ifdef _WIN32
  return;
#else
  if (model_.preview_log_pipe_read_fd < 0)
  {
    return;
  }

  fflush(stdout);
  fflush(stderr);

  if (model_.preview_log_saved_stdout_fd >= 0)
  {
    dup2(model_.preview_log_saved_stdout_fd, STDOUT_FILENO);
    close(model_.preview_log_saved_stdout_fd);
    model_.preview_log_saved_stdout_fd = -1;
  }
  if (model_.preview_log_saved_stderr_fd >= 0)
  {
    dup2(model_.preview_log_saved_stderr_fd, STDERR_FILENO);
    close(model_.preview_log_saved_stderr_fd);
    model_.preview_log_saved_stderr_fd = -1;
  }

  PumpGameLogOutput();
  if (!model_.preview_log_partial_line.empty())
  {
    model_.game_log_lines.push_back(model_.preview_log_partial_line);
    model_.game_log_scroll_to_bottom = true;
    model_.preview_log_partial_line.clear();
  }

  if (model_.preview_log_pipe_write_fd >= 0)
  {
    close(model_.preview_log_pipe_write_fd);
    model_.preview_log_pipe_write_fd = -1;
  }
  if (model_.preview_log_pipe_read_fd >= 0)
  {
    close(model_.preview_log_pipe_read_fd);
    model_.preview_log_pipe_read_fd = -1;
  }

  model_.game_log_lines.push_back("[editor] preview log capture stopped");
  model_.game_log_scroll_to_bottom = true;
#endif
}

void MyriadEditor::RefreshPaths(bool force_defaults)
{
  if (model_.compiler_presets.empty())
  {
    model_.compiler_presets.push_back({"Default", {}});
  }

  const auto &preset = model_.compiler_presets[std::max(0, std::min(model_.selected_preset_index, static_cast<int>(model_.compiler_presets.size()) - 1))];
  const std::filesystem::path detected_build_dir = Editor::FindMatchingBuildDirectory(model_.project_root, preset.name);
  const std::string build_type = model_.settings.build_type.empty() ? std::string{"Debug"} : model_.settings.build_type;
  const std::filesystem::path default_build_dir = detected_build_dir.empty() ? Editor::ResolvePath(model_.project_root, std::filesystem::path("build") / preset.name / build_type) : detected_build_dir;

  std::filesystem::path selected_build_dir = force_defaults || model_.build_dir_input.empty() ? default_build_dir : ResolveProjectPathText(model_.project_root, model_.build_dir_input);
  if (!selected_build_dir.is_absolute())
  {
    selected_build_dir = Editor::ResolvePath(model_.project_root, selected_build_dir);
  }

  model_.build_dir = std::filesystem::absolute(selected_build_dir);
  model_.build_dir_input = ProjectRelativePathText(model_.project_root, model_.build_dir);
  model_.settings.build_directory = model_.build_dir_input;
  model_.settings.last_build_dir = model_.settings.build_directory;

  if (force_defaults || model_.executable_input.empty())
  {
    model_.game_executable = Editor::FindGameExecutable(model_.build_dir, {}, model_.project_root, model_.settings.target_executable_name);
  }
  else
  {
    const std::filesystem::path resolved_override = Editor::ResolveExecutablePath(model_.build_dir, model_.project_root, std::filesystem::path(model_.executable_input));
    model_.game_executable = Editor::FindGameExecutable(model_.build_dir, resolved_override.empty() ? std::filesystem::path{} : resolved_override, model_.project_root, model_.settings.target_executable_name);
  }

  model_.executable_input = model_.game_executable.empty() ? "" : model_.game_executable.string();
  model_.status = "Paths refreshed for the selected compiler toolkit and build type.";
}

void MyriadEditor::RefreshBuildBridgeStatus(bool force)
{
  if (!Editor::ShouldUseSocketBuilds(model_.settings))
  {
    Editor::DisconnectBuildBridge();
    model_.build_bridge_connected = false;
    model_.build_bridge_status_text = "Local build mode.";
    model_.build_bridge_consecutive_failures = 0;
    model_.build_bridge_warning_active = false;
    model_.build_bridge_last_probe_time = GetTime();
    return;
  }

  const double now = GetTime();
  const double probe_interval_seconds = static_cast<double>(std::max(5, std::min(3600, model_.settings.build_bridge_probe_interval_seconds)));
  if (!force)
  {
    if (now - model_.build_bridge_last_probe_time < probe_interval_seconds)
    {
      return;
    }
  }

  model_.build_bridge_last_probe_time = now;
  std::string response;
  std::string bridge_error;
  if (Editor::ProbeBuildBridge(model_.settings, response, bridge_error))
  {
    const bool bridge_success = Editor::ExtractJsonBool(response, "success", true);
    const std::string bridge_status = Editor::ExtractJsonString(response, "status");
    const bool rebuild_needed = Editor::ExtractJsonBool(response, "rebuildNeeded", false);
    const int changed_file_count = Editor::ExtractJsonInt(response, "changedFileCount", 0);
    const std::vector<std::string> changed_files_preview = ExtractJsonStringArray(response, "changedFilesPreview", 12);

    const bool had_rebuild_needed = model_.bridge_rebuild_needed;
    model_.bridge_rebuild_needed = rebuild_needed;
    model_.bridge_changed_file_count = std::max(0, changed_file_count);
    model_.bridge_changed_files_preview = changed_files_preview;

    model_.build_bridge_connected = true;
    model_.build_bridge_status_text = bridge_status.empty() ? "Bridge reachable." : bridge_status;
    (void)bridge_success;
    if (model_.build_bridge_warning_active)
    {
      AppendConsoleLine("Build bridge connectivity restored.");
    }
    model_.build_bridge_consecutive_failures = 0;
    model_.build_bridge_warning_active = false;

    if (!had_rebuild_needed && model_.bridge_rebuild_needed)
    {
      AppendConsoleLine("Build bridge detected source changes. Rebuild is needed.");
    }
    else if (had_rebuild_needed && !model_.bridge_rebuild_needed)
    {
      AppendConsoleLine("Build bridge reports outputs are up to date.");
    }

    if (force)
    {
      AppendConsoleLine("Bridge status: " + model_.build_bridge_status_text);
    }
  }
  else
  {
    model_.build_bridge_connected = false;
    model_.bridge_rebuild_needed = false;
    model_.bridge_changed_file_count = 0;
    model_.bridge_changed_files_preview.clear();
    model_.build_bridge_status_text = bridge_error;
    ++model_.build_bridge_consecutive_failures;
    if (force)
    {
      AppendConsoleLine("Bridge status error: " + bridge_error);
    }
  }

  constexpr int warning_threshold = 3;
  if (!model_.build_bridge_connected && model_.build_bridge_consecutive_failures >= warning_threshold)
  {
    if (!model_.build_bridge_warning_active)
    {
      AppendConsoleLine("Warning: build bridge probe failed repeatedly; startup/builds may be degraded until connectivity is restored.");
    }
    model_.build_bridge_warning_active = true;
  }
}

void MyriadEditor::RefreshProjectBrowser(const std::string &relative_path)
{
  std::string response;
  std::string bridge_error;
  const std::string request = Editor::CreateProjectListRequest(relative_path);
  if (!Editor::SendBuildBridgeRequestWithFallback(model_.settings, request, response, bridge_error, nullptr, 1000, 2000))
  {
    model_.project_browser_status = "Project browser error: " + bridge_error;
    AppendConsoleLine(model_.project_browser_status);
    return;
  }

  const bool success = Editor::ExtractJsonBool(response, "success", false);
  model_.project_browser_status = Editor::ExtractJsonString(response, "status");
  if (!success)
  {
    if (model_.project_browser_status.empty())
    {
      model_.project_browser_status = "Project browser request failed.";
    }
    AppendConsoleLine(model_.project_browser_status);
    return;
  }

  model_.project_browser_mount_path = Editor::ExtractJsonString(response, "projectMount");
  const std::string bridge_project_mount_source = Editor::ExtractJsonString(response, "projectMountSource");
  if (!bridge_project_mount_source.empty())
  {
    model_.project_mount_source_path = bridge_project_mount_source;
  }
  model_.project_browser_relative_path = Editor::ExtractJsonString(response, "path");
  model_.project_browser_directories = ExtractJsonStringArray(response, "directories", 256);
  if (model_.project_browser_status.empty())
  {
    model_.project_browser_status = "Projects listed.";
  }
}

void MyriadEditor::RefreshExportDirectoryBrowser(const std::string &relative_path)
{
  std::string response;
  std::string bridge_error;
  const std::string request = Editor::CreateDirectoryListRequest(relative_path);
  if (!Editor::SendBuildBridgeRequestWithFallback(model_.settings, request, response, bridge_error, nullptr, 1000, 2000))
  {
    model_.export_browser_status = "Export directory browser error: " + bridge_error;
    AppendConsoleLine(model_.export_browser_status);
    return;
  }

  const bool success = Editor::ExtractJsonBool(response, "success", false);
  model_.export_browser_status = Editor::ExtractJsonString(response, "status");
  if (!success)
  {
    if (model_.export_browser_status.empty())
    {
      model_.export_browser_status = "Export directory browser request failed.";
    }
    AppendConsoleLine(model_.export_browser_status);
    return;
  }

  const std::string bridge_project_mount = Editor::ExtractJsonString(response, "projectMount");
  const std::string bridge_project_mount_source = Editor::ExtractJsonString(response, "projectMountSource");
  if (!bridge_project_mount.empty())
  {
    model_.project_browser_mount_path = bridge_project_mount;
  }
  if (!bridge_project_mount_source.empty())
  {
    model_.project_mount_source_path = bridge_project_mount_source;
  }
  model_.export_browser_relative_path = Editor::ExtractJsonString(response, "path");
  model_.export_browser_directories = ExtractJsonStringArray(response, "directories", 256);
  if (model_.export_browser_status.empty())
  {
    model_.export_browser_status = "Directories listed.";
  }
}

void MyriadEditor::RefreshSourceDirectoryBrowser(const std::string &relative_path)
{
  model_.source_browser_directories.clear();
  model_.source_browser_relative_path = NormalizeProjectBrowserRelativePath(relative_path);
  if (model_.project_root.empty())
  {
    model_.source_browser_status = "Select a project root before browsing source directories.";
    return;
  }

  const std::filesystem::path directory_path = model_.project_root / model_.source_browser_relative_path;
  std::error_code error_code;
  if (!std::filesystem::exists(directory_path, error_code) || !std::filesystem::is_directory(directory_path, error_code))
  {
    model_.source_browser_status = "Source directory not found: " + directory_path.string();
    return;
  }

  for (const auto &entry : std::filesystem::directory_iterator(directory_path, error_code))
  {
    if (error_code)
    {
      break;
    }
    std::error_code entry_error_code;
    if (!entry.is_directory(entry_error_code))
    {
      continue;
    }

    const std::string name = entry.path().filename().generic_string();
    if (name.empty() || name.front() == '.')
    {
      continue;
    }
    model_.source_browser_directories.push_back(name);
  }

  std::sort(model_.source_browser_directories.begin(), model_.source_browser_directories.end());
  model_.source_browser_status = error_code ? "Source directory browser error: " + error_code.message() : "Directories listed.";
}

void MyriadEditor::RefreshResourcesDirectoryBrowser(const std::string &relative_path)
{
  model_.resources_browser_directories.clear();
  model_.resources_browser_relative_path = NormalizeProjectBrowserRelativePath(relative_path);
  if (model_.project_root.empty())
  {
    model_.resources_browser_status = "Select a project root before browsing resources directories.";
    return;
  }

  const std::filesystem::path directory_path = model_.project_root / model_.resources_browser_relative_path;
  std::error_code error_code;
  if (!std::filesystem::exists(directory_path, error_code) || !std::filesystem::is_directory(directory_path, error_code))
  {
    model_.resources_browser_status = "Resources directory not found: " + directory_path.string();
    return;
  }

  for (const auto &entry : std::filesystem::directory_iterator(directory_path, error_code))
  {
    if (error_code)
    {
      break;
    }
    std::error_code entry_error_code;
    if (!entry.is_directory(entry_error_code))
    {
      continue;
    }

    const std::string name = entry.path().filename().generic_string();
    if (name.empty() || name.front() == '.')
    {
      continue;
    }
    model_.resources_browser_directories.push_back(name);
  }

  std::sort(model_.resources_browser_directories.begin(), model_.resources_browser_directories.end());
  model_.resources_browser_status = error_code ? "Resources directory browser error: " + error_code.message() : "Directories listed.";
}

void MyriadEditor::RefreshBuildOptions()
{
  if (model_.build_type_options.empty())
  {
    model_.build_type_options = {"Debug", "Release", "RelWithDebInfo", "MinSizeRel"};
  }

  if (!Editor::ShouldUseSocketBuilds(model_.settings))
  {
    return;
  }

  std::string response;
  std::string bridge_error;
  const std::string request = Editor::CreateBuildOptionsRequest(model_.settings.project_mount_path, model_.settings.compiler_toolkit, model_.settings.build_type);
  if (!Editor::SendBuildBridgeRequestWithFallback(model_.settings, request, response, bridge_error, nullptr, 1000, 2000))
  {
    AppendConsoleLine("Build options bridge error: " + bridge_error);
    return;
  }

  if (!Editor::ExtractJsonBool(response, "success", false))
  {
    const std::string bridge_status = Editor::ExtractJsonString(response, "status");
    AppendConsoleLine(bridge_status.empty() ? "Build options request failed." : bridge_status);
    return;
  }

  const std::string bridge_project_mount = Editor::ExtractJsonString(response, "projectMount");
  const std::string bridge_project_mount_source = Editor::ExtractJsonString(response, "projectMountSource");
  const std::string bridge_dist_mount_source = Editor::ExtractJsonString(response, "distMountSource");
  if (!bridge_project_mount.empty())
  {
    model_.project_browser_mount_path = bridge_project_mount;
  }
  if (!bridge_project_mount_source.empty())
  {
    model_.project_mount_source_path = bridge_project_mount_source;
  }
  if (!bridge_dist_mount_source.empty())
  {
    model_.dist_mount_source_path = bridge_dist_mount_source;
  }

  const std::string bridge_header_default = Editor::ExtractJsonString(response, "defaultHeaderSearchDirs");
  const std::string bridge_library_default = Editor::ExtractJsonString(response, "defaultLibrarySearchDirs");
  if (!bridge_header_default.empty())
  {
    model_.default_header_search_dirs = bridge_header_default;
  }
  if (!bridge_library_default.empty())
  {
    model_.default_library_search_dirs = bridge_library_default;
  }

  const std::vector<std::string> bridge_toolkits = ExtractJsonStringArray(response, "compilerToolkits", 128);
  if (!bridge_toolkits.empty())
  {
    model_.compiler_presets.clear();
    for (const auto &toolkit : bridge_toolkits)
    {
      model_.compiler_presets.push_back({toolkit, {}});
    }
  }

  const std::vector<std::string> bridge_build_types = ExtractJsonStringArray(response, "buildTypes", 16);
  if (!bridge_build_types.empty())
  {
    model_.build_type_options = bridge_build_types;
  }

  if (model_.compiler_presets.empty())
  {
    model_.compiler_presets.push_back({"Default", {}});
  }

  model_.selected_preset_index = 0;
  for (int i = 0; i < static_cast<int>(model_.compiler_presets.size()); ++i)
  {
    if (model_.compiler_presets[i].name == model_.settings.compiler_toolkit)
    {
      model_.selected_preset_index = i;
      break;
    }
  }

  const std::string selected_bridge_toolkit = Editor::ExtractJsonString(response, "compilerToolkit");
  if (!selected_bridge_toolkit.empty() && model_.settings.compiler_toolkit.empty())
  {
    model_.settings.compiler_toolkit = selected_bridge_toolkit;
  }

  if (model_.settings.build_type.empty() || std::find(model_.build_type_options.begin(), model_.build_type_options.end(), model_.settings.build_type) == model_.build_type_options.end())
  {
    const std::string selected_bridge_build_type = Editor::ExtractJsonString(response, "buildType");
    model_.settings.build_type = !selected_bridge_build_type.empty() ? selected_bridge_build_type : (model_.build_type_options.empty() ? std::string{"Debug"} : model_.build_type_options.front());
  }
}

void MyriadEditor::OpenBridgeProject(const std::string &relative_path)
{
  std::string response;
  std::string bridge_error;
  const std::string request = Editor::CreateProjectOpenRequest(relative_path);
  if (!Editor::SendBuildBridgeRequestWithFallback(model_.settings, request, response, bridge_error, nullptr, 1000, 3000))
  {
    model_.status = "Open project bridge error: " + bridge_error;
    AppendConsoleLine(model_.status);
    return;
  }

  const bool success = Editor::ExtractJsonBool(response, "success", false);
  const std::string bridge_status = Editor::ExtractJsonString(response, "status");
  if (!success)
  {
    model_.status = bridge_status.empty() ? "The build bridge could not open the selected project." : bridge_status;
    AppendConsoleLine(model_.status);
    return;
  }

  const std::string project_root_text = Editor::ExtractJsonString(response, "projectRoot");
  const std::string project_display_root_text = Editor::ExtractJsonString(response, "projectDisplayRoot");
  const std::string project_mount_path = Editor::ExtractJsonString(response, "projectPath");
  const std::string bridge_project_mount = Editor::ExtractJsonString(response, "projectMount");
  const std::string bridge_project_mount_source = Editor::ExtractJsonString(response, "projectMountSource");
  if (!bridge_project_mount.empty())
  {
    model_.project_browser_mount_path = bridge_project_mount;
  }
  if (!bridge_project_mount_source.empty())
  {
    model_.project_mount_source_path = bridge_project_mount_source;
  }
  if (project_root_text.empty())
  {
    model_.status = "The build bridge did not return a project root.";
    AppendConsoleLine(model_.status);
    return;
  }

  ApplyProjectDirectory(SelectSourceProjectRoot(project_root_text, project_display_root_text), project_mount_path, true);
  model_.status = bridge_status.empty() ? "Project opened." : bridge_status;
  AppendConsoleLine("Opened project: " + model_.project_root.string());
}

void MyriadEditor::CreateBridgeProject(const std::string &parent_path, const std::string &project_name)
{
  const std::string trimmed_name = Editor::Trim(project_name);
  if (trimmed_name.empty())
  {
    model_.status = "New project name is required.";
    AppendConsoleLine(model_.status);
    return;
  }

  std::string response;
  std::string bridge_error;
  const std::string request = Editor::CreateProjectCreateRequest(parent_path, trimmed_name);
  if (!Editor::SendBuildBridgeRequestWithFallback(model_.settings, request, response, bridge_error, nullptr, 1000, 5000))
  {
    model_.status = "Create project bridge error: " + bridge_error;
    AppendConsoleLine(model_.status);
    return;
  }

  const bool success = Editor::ExtractJsonBool(response, "success", false);
  const std::string bridge_status = Editor::ExtractJsonString(response, "status");
  if (!success)
  {
    model_.status = bridge_status.empty() ? "The build bridge could not create the project." : bridge_status;
    AppendConsoleLine(model_.status);
    return;
  }

  const std::string project_root_text = Editor::ExtractJsonString(response, "projectRoot");
  const std::string project_display_root_text = Editor::ExtractJsonString(response, "projectDisplayRoot");
  const std::string project_mount_path = Editor::ExtractJsonString(response, "projectPath");
  const std::string bridge_project_mount = Editor::ExtractJsonString(response, "projectMount");
  const std::string bridge_project_mount_source = Editor::ExtractJsonString(response, "projectMountSource");
  if (!bridge_project_mount.empty())
  {
    model_.project_browser_mount_path = bridge_project_mount;
  }
  if (!bridge_project_mount_source.empty())
  {
    model_.project_mount_source_path = bridge_project_mount_source;
  }
  if (project_root_text.empty())
  {
    model_.status = "The build bridge did not return a project root for the new project.";
    AppendConsoleLine(model_.status);
    return;
  }

  ApplyProjectDirectory(SelectSourceProjectRoot(project_root_text, project_display_root_text), project_mount_path, true);
  RefreshProjectBrowser(parent_path);
  model_.new_project_name.clear();
  model_.status = bridge_status.empty() ? "Project created." : bridge_status;
  AppendConsoleLine("Created project: " + model_.project_root.string());
}

bool MyriadEditor::CreateBridgeDirectory(const std::string &parent_path, const std::string &directory_name, std::string &created_relative_path)
{
  const std::string trimmed_name = Editor::Trim(directory_name);
  if (trimmed_name.empty())
  {
    model_.status = "New directory name is required.";
    model_.export_browser_status = model_.status;
    AppendConsoleLine(model_.status);
    return false;
  }

  std::string response;
  std::string bridge_error;
  const std::string request = Editor::CreateDirectoryCreateRequest(parent_path, trimmed_name);
  if (!Editor::SendBuildBridgeRequestWithFallback(model_.settings, request, response, bridge_error, nullptr, 1000, 5000))
  {
    model_.status = "Create directory bridge error: " + bridge_error;
    model_.export_browser_status = model_.status;
    AppendConsoleLine(model_.status);
    return false;
  }

  const bool success = Editor::ExtractJsonBool(response, "success", false);
  const std::string bridge_status = Editor::ExtractJsonString(response, "status");
  if (!success)
  {
    model_.status = bridge_status.empty() ? "The build bridge could not create the directory." : bridge_status;
    model_.export_browser_status = model_.status;
    AppendConsoleLine(model_.status);
    return false;
  }

  created_relative_path = Editor::ExtractJsonString(response, "path");
  model_.status = bridge_status.empty() ? "Directory created." : bridge_status;
  model_.export_browser_status = model_.status;
  AppendConsoleLine("Created directory: " + JoinSourceMountPath(model_.project_mount_source_path, created_relative_path));
  return true;
}

void MyriadEditor::ApplyProjectDirectory(const std::filesystem::path &project_root, const std::string &project_mount_path, bool reload_project_settings)
{
  Editor::EditorSettings previous_settings = model_.settings;
  model_.project_root = std::filesystem::absolute(project_root);

  if (reload_project_settings)
  {
    model_.settings = Editor::LoadEditorSettings(model_.project_root);
    if (model_.settings.build_mode.empty())
    {
      model_.settings.build_mode = previous_settings.build_mode;
    }
    if (model_.settings.build_socket_host.empty())
    {
      model_.settings.build_socket_host = previous_settings.build_socket_host;
    }
    if (model_.settings.build_socket_hosts.empty())
    {
      model_.settings.build_socket_hosts = previous_settings.build_socket_hosts;
    }
    if (model_.settings.build_socket_port == 55333 && previous_settings.build_socket_port != 55333)
    {
      model_.settings.build_socket_port = previous_settings.build_socket_port;
    }
  }

  model_.settings.project_root_path = model_.project_root.string();
  model_.settings.project_mount_path = project_mount_path;
  if (model_.settings.compiler_toolkit.empty())
  {
    model_.settings.compiler_toolkit = model_.settings.last_compiler_preset;
  }
  if (model_.settings.build_type.empty())
  {
    model_.settings.build_type = "Debug";
  }
  if (model_.settings.build_profiles.empty())
  {
    model_.settings.selected_build_profile = model_.settings.selected_build_profile.empty() ? std::string{"Default"} : model_.settings.selected_build_profile;
    model_.settings.build_profiles.push_back(CaptureBuildProfileFromSettings(model_.settings, model_.settings.last_executable_path));
  }
  model_.selected_build_profile_index = BuildProfileIndexByName(model_.settings.build_profiles, model_.settings.selected_build_profile);
  if (model_.selected_build_profile_index < 0)
  {
    model_.selected_build_profile_index = 0;
    ApplyBuildProfileToSettings(model_.settings.build_profiles.front(), model_.settings);
  }
  model_.build_command_template = Editor::ResolveBuildCommandTemplate(model_.settings);
  model_.compiler_presets = Editor::LoadCompilerPresets(model_.project_root);
  if (model_.compiler_presets.empty())
  {
    model_.compiler_presets.push_back({"Default", {}});
  }
  model_.selected_preset_index = 0;
  if (!model_.settings.compiler_toolkit.empty())
  {
    for (int i = 0; i < static_cast<int>(model_.compiler_presets.size()); ++i)
    {
      if (model_.compiler_presets[i].name == model_.settings.compiler_toolkit)
      {
        model_.selected_preset_index = i;
        break;
      }
    }
  }
  model_.last_selected_preset_index = -1;
  model_.theme_presets = LoadThemePresets(model_.project_root);
  model_.layout_presets = LoadLayoutPresets(model_.project_root);
  model_.selected_theme_preset_index = ThemePresetIndexFromName(model_.settings.theme_preset, model_.theme_presets);
  model_.selected_layout_preset_index = std::max(0, std::min(model_.settings.layout_preset_index, static_cast<int>(model_.layout_presets.size()) - 1));
  model_.ui_font_scale = static_cast<float>(model_.settings.ui_font_scale_percent) / 100.0f;
  model_.ui_rounding = static_cast<float>(model_.settings.ui_rounding);
  model_.ui_spacing_density = static_cast<float>(model_.settings.ui_spacing_percent) / 100.0f;
  if (!ParseHexColor(model_.settings.ui_accent_hex, model_.ui_accent_color))
  {
    model_.ui_accent_color = ImVec4(0.30f, 0.54f, 0.81f, 1.0f);
  }
  ApplyThemePresetByIndex(model_.selected_theme_preset_index);
  ApplyThemeCustomizations();
  RefreshBuildOptions();
  RefreshPaths(true);
  const bool search_dir_defaults_applied = ApplySearchDirDefaults(model_.settings, model_.default_header_search_dirs, model_.default_library_search_dirs);
  model_.dock_layout_apply_requested = true;
  model_.project_settings_dirty = search_dir_defaults_applied;
  model_.build_bridge_last_probe_time = 0.0;
  RefreshBuildBridgeStatus(true);
}

void MyriadEditor::BuildTestECS()
{
  model_.build_succeeded = false;
  const std::filesystem::path configured_project_root = ResolveConfiguredProjectDirectory(model_.settings.project_root_path);
  if (!configured_project_root.empty())
  {
    model_.project_root = configured_project_root;
    model_.settings.project_root_path = model_.project_root.string();
  }

  if (model_.project_root.empty())
  {
    model_.build_succeeded = false;
    model_.status = "Set a project directory in Editor Preferences before building.";
    AppendConsoleLine(model_.status);
    return;
  }

  const auto &preset = model_.compiler_presets[std::max(0, std::min(model_.selected_preset_index, static_cast<int>(model_.compiler_presets.size()) - 1))];
  const std::filesystem::path detected_build_dir = Editor::FindMatchingBuildDirectory(model_.project_root, preset.name);
  const std::string build_type = model_.settings.build_type.empty() ? std::string{"Debug"} : model_.settings.build_type;
  const std::filesystem::path default_build_dir = detected_build_dir.empty() ? (model_.project_root / "build" / preset.name / build_type) : detected_build_dir;
  model_.build_dir = std::filesystem::absolute(model_.build_dir_input.empty() ? default_build_dir : ResolveProjectPathText(model_.project_root, model_.build_dir_input));
  model_.build_dir_input = ProjectRelativePathText(model_.project_root, model_.build_dir);
  model_.settings.build_directory = model_.build_dir_input;
  model_.settings.last_build_dir = model_.settings.build_directory;
  std::filesystem::create_directories(model_.build_dir);

  if (Editor::ShouldUseSocketBuilds(model_.settings))
  {
    const std::string build_dir_relative = Editor::BuildDirForBridge(model_.build_dir, model_.project_root);
    if (build_dir_relative.empty())
    {
      model_.build_succeeded = false;
      model_.status = "Socket builds require the build directory to be inside the project root.";
      AppendConsoleLine(model_.status);
      return;
    }

    StartBridgeBuildOverSocket(preset, build_dir_relative);
    return;
  }

  std::filesystem::path toolchain_path;
  if (!preset.toolchain_file.empty())
  {
    const std::filesystem::path resolved_toolchain_path = Editor::ResolvePath(model_.project_root, preset.toolchain_file);
    if (std::filesystem::exists(resolved_toolchain_path))
    {
      toolchain_path = resolved_toolchain_path;
    }
  }

  std::string command = Editor::ExpandBuildCommandTemplate(model_.build_command_template,
                                                           model_.project_root,
                                                           model_.build_dir,
                                                           preset,
                                                           model_.settings.build_type,
                                                           toolchain_path,
                                                           model_.settings.target_executable_name,
                                                           model_.settings.header_search_dirs,
                                                           model_.settings.library_search_dirs);
  std::cout << "Building " << (model_.settings.target_executable_name.empty() ? std::string{"TestECS"} : model_.settings.target_executable_name) << " with: " << command << std::endl;
  model_.status = "Building " + (model_.settings.target_executable_name.empty() ? std::string{"TestECS"} : model_.settings.target_executable_name) + "...";
  AppendConsoleLine("Build request: " + preset.name + " -> " + model_.build_dir.string());

  const int result = std::system(command.c_str());
  if (result == 0)
  {
    model_.build_succeeded = true;
    model_.bridge_rebuild_needed = false;
    model_.bridge_changed_file_count = 0;
    const std::filesystem::path detected_build_dir = Editor::FindMatchingBuildDirectory(model_.project_root, preset.name);
    const std::string build_type = model_.settings.build_type.empty() ? std::string{"Debug"} : model_.settings.build_type;
    const std::filesystem::path default_build_dir = detected_build_dir.empty() ? (model_.project_root / "build" / preset.name / build_type) : detected_build_dir;
    model_.build_dir = std::filesystem::absolute(model_.build_dir_input.empty() ? default_build_dir : ResolveProjectPathText(model_.project_root, model_.build_dir_input));
    model_.build_dir_input = ProjectRelativePathText(model_.project_root, model_.build_dir);
    model_.settings.build_directory = model_.build_dir_input;
    model_.settings.last_build_dir = model_.settings.build_directory;
    model_.game_executable = Editor::FindGameExecutable(model_.build_dir, model_.executable_input, model_.project_root, model_.settings.target_executable_name);
    model_.executable_input = model_.game_executable.empty() ? model_.executable_input : model_.game_executable.string();
    model_.status = "Build succeeded.";
    AppendConsoleLine("Build succeeded.");
    RestartPreviewForLatestBuild();
  }
  else
  {
    model_.build_succeeded = false;
    model_.status = "Build failed. Check the terminal output for details.";
    AppendConsoleLine("Build failed. Check the terminal output for details.");
  }
}

void MyriadEditor::RunTestECS()
{
  if (model_.game_pid > 0)
  {
    model_.status = "The " + (model_.settings.target_executable_name.empty() ? std::string{"TestECS"} : model_.settings.target_executable_name) + " process is already running.";
    return;
  }

  const std::filesystem::path existing_executable = model_.game_executable;
  if (existing_executable.empty() || !std::filesystem::exists(existing_executable))
  {
    const auto &preset = model_.compiler_presets[std::max(0, std::min(model_.selected_preset_index, static_cast<int>(model_.compiler_presets.size()) - 1))];
    const std::filesystem::path detected_build_dir = Editor::FindMatchingBuildDirectory(model_.project_root, preset.name);
    const std::string build_type = model_.settings.build_type.empty() ? std::string{"Debug"} : model_.settings.build_type;
    const std::filesystem::path default_build_dir = detected_build_dir.empty() ? (model_.project_root / "build" / preset.name / build_type) : detected_build_dir;
    model_.build_dir = std::filesystem::absolute(model_.build_dir_input.empty() ? default_build_dir : ResolveProjectPathText(model_.project_root, model_.build_dir_input));
    model_.build_dir_input = ProjectRelativePathText(model_.project_root, model_.build_dir);
    model_.settings.build_directory = model_.build_dir_input;
    model_.settings.last_build_dir = model_.settings.build_directory;
    model_.game_executable = Editor::FindGameExecutable(model_.build_dir, model_.executable_input, model_.project_root, model_.settings.target_executable_name);
  }

  if (model_.game_executable.empty())
  {
    model_.status = "Unable to locate the " + (model_.settings.target_executable_name.empty() ? std::string{"TestECS"} : model_.settings.target_executable_name) + " executable. Build it first.";
    return;
  }

  std::intptr_t process_id = -1;
  const std::filesystem::path runtime_log_dir = model_.project_root / "runtime_logs";
  std::error_code runtime_log_error;
  std::filesystem::create_directories(runtime_log_dir, runtime_log_error);
  const std::filesystem::path runtime_log_path = runtime_log_dir / ("game-runtime-" + TimestampForLogPath() + ".log");
  const std::vector<std::filesystem::path> runtime_library_dirs = RuntimeLibraryDirectories(model_.game_executable, model_.build_dir);
  const std::filesystem::path configured_resources_dir = ResolveConfiguredResourcesDirectory(model_.project_root, model_.settings.resources_directory);
  std::vector<std::pair<std::string, std::string>> environment_overrides;
  if (!configured_resources_dir.empty())
  {
    std::error_code resources_error;
    if (!std::filesystem::exists(configured_resources_dir, resources_error) || !std::filesystem::is_directory(configured_resources_dir, resources_error))
    {
      model_.status = "Resources directory not found: " + configured_resources_dir.string();
      AppendConsoleLine(model_.status);
      return;
    }
    environment_overrides.push_back({"MYRIAD_RESOURCE_BASE_PATH", model_.settings.resources_directory});
  }

  if (Editor::LaunchProcess(model_.game_executable, model_.project_root, process_id, &runtime_log_path, &runtime_library_dirs, &environment_overrides))
  {
    model_.game_pid = process_id;
    model_.runtime_log_path = runtime_log_path;
    model_.runtime_log_offset = 0;
    model_.game_log_lines.clear();
    model_.game_log_lines.push_back("[editor] runtime log: " + runtime_log_path.string());
    model_.game_log_scroll_to_bottom = true;
    model_.show_game_log_window = true;
    model_.status = (model_.settings.target_executable_name.empty() ? std::string{"TestECS"} : model_.settings.target_executable_name) + " launched.";
    AppendConsoleLine("Launched game runtime log: " + runtime_log_path.string());
    if (!configured_resources_dir.empty())
    {
      AppendConsoleLine("Runtime resources directory: " + configured_resources_dir.string());
    }
  }
  else
  {
    model_.status = "Failed to launch " + (model_.settings.target_executable_name.empty() ? std::string{"TestECS"} : model_.settings.target_executable_name) + ".";
  }
}

void MyriadEditor::StopGame()
{
  if (model_.game_pid > 0)
  {
    Editor::StopProcess(model_.game_pid);
    model_.game_pid = -1;
    model_.status = "Stopped the game process.";
  }
}
