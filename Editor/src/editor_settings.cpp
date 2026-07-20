#include "editor_settings.h"

#include "json_utils.h"
#include "project_paths.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <ostream>
#include <sstream>

namespace Editor
{
  namespace
  {
    std::vector<std::string> ParseHostList(const std::string &host_list);

    std::filesystem::path ResolveEditorConfigOverridePath()
    {
      const char *config_path_override = std::getenv("MYRIAD_EDITOR_CONFIG_PATH");
      if (config_path_override != nullptr && *config_path_override != '\0')
      {
        return std::filesystem::path(config_path_override);
      }

      return {};
    }

    std::filesystem::path ResolveEditorResourceConfigPath(const std::filesystem::path &root)
    {
      const std::filesystem::path installed_data_dir = GetInstalledEditorDataDirectory();
      if (!installed_data_dir.empty())
      {
        return installed_data_dir / ".myriad-editor.json";
      }

      if (!root.empty())
      {
        const auto legacy_candidate = root.parent_path() / "share" / "myriad-editor" / ".myriad-editor.json";
        if (std::filesystem::exists(legacy_candidate.parent_path()))
        {
          return legacy_candidate;
        }
      }
      return {};
    }

    bool IsInstalledEditorRun(const std::filesystem::path &root)
    {
      if (root.empty())
      {
        return true;
      }

      const std::filesystem::path executable_dir = GetExecutableDirectory();
      if (executable_dir.empty())
      {
        return false;
      }

      std::error_code error_code;
      const std::filesystem::path relative_executable_dir = std::filesystem::relative(executable_dir, root, error_code);
      if (error_code || relative_executable_dir.empty())
      {
        return true;
      }

      const std::string relative_text = relative_executable_dir.generic_string();
      return relative_text.rfind("..", 0) == 0;
    }

    std::filesystem::path ResolveSettingsReadPath(const std::filesystem::path &root)
    {
      const std::filesystem::path override_settings = ResolveEditorConfigOverridePath();
      if (!override_settings.empty() && std::filesystem::exists(override_settings))
      {
        return override_settings;
      }

      if (!root.empty())
      {
        const std::filesystem::path project_settings = root / ".myriad_editor" / "editor.json";
        if (std::filesystem::exists(project_settings))
        {
          return project_settings;
        }

        const std::filesystem::path legacy_project_settings = root / ".myriad-editor.json";
        if (std::filesystem::exists(legacy_project_settings))
        {
          return legacy_project_settings;
        }
      }

      const std::filesystem::path resource_settings = ResolveEditorResourceConfigPath(root);
      if (!resource_settings.empty())
      {
        return resource_settings;
      }

      return {};
    }

    std::filesystem::path ResolveSettingsWritePath(const std::filesystem::path &root)
    {
      const std::filesystem::path override_settings = ResolveEditorConfigOverridePath();
      if (!override_settings.empty())
      {
        return override_settings;
      }

      if (!root.empty())
      {
        return root / ".myriad_editor" / "editor.json";
      }

      return ResolveEditorResourceConfigPath(root);
    }

    std::string JoinHostList(const std::vector<std::string> &hosts)
    {
      std::ostringstream joined;
      for (std::size_t i = 0; i < hosts.size(); ++i)
      {
        if (i > 0)
        {
          joined << ',';
        }
        joined << hosts[i];
      }
      return joined.str();
    }

    std::string ResolveWslBridgeHost()
    {
      std::ifstream resolv_conf("/etc/resolv.conf");
      if (!resolv_conf.is_open())
      {
        return {};
      }

      std::string line;
      while (std::getline(resolv_conf, line))
      {
        std::istringstream line_stream(line);
        std::string keyword;
        std::string host;
        if (!(line_stream >> keyword >> host))
        {
          continue;
        }

        if (keyword == "nameserver" && !host.empty() && host != "127.0.0.1" && host != "::1")
        {
          return host;
        }
      }

      return {};
    }

    std::string PathStatusLine(const std::string &label, const std::filesystem::path &path)
    {
      if (path.empty())
      {
        return label + ": <none>";
      }

      std::error_code error_code;
      const bool exists = std::filesystem::exists(path, error_code);
      std::string status = label + ": " + path.string() + " [";
      if (error_code)
      {
        status += "error: " + error_code.message();
      }
      else
      {
        status += exists ? "found" : "missing";
      }
      status += "]";
      return status;
    }

    std::string BoolStatusLine(const std::string &label, bool value)
    {
      return label + ": " + (value ? "true" : "false");
    }

    std::string ProjectRelativePathText(const std::filesystem::path &project_root, const std::string &path_text)
    {
      const std::string trimmed_path = Trim(path_text);
      if (trimmed_path.empty())
      {
        return {};
      }

      const std::filesystem::path path(trimmed_path);
      if (!path.is_absolute() || project_root.empty())
      {
        return path.generic_string();
      }

      std::error_code error_code;
      const std::filesystem::path relative_path = std::filesystem::relative(path, project_root, error_code);
      if (!error_code && !relative_path.empty())
      {
        const std::string relative_text = relative_path.generic_string();
        if (relative_text == ".")
        {
          return {};
        }
        if (relative_text != ".." && relative_text.rfind("../", 0) != 0)
        {
          return relative_text;
        }
      }

      return path.generic_string();
    }

    std::string ExtractJsonArrayText(const std::string &text, const std::string &key)
    {
      const std::string pattern = "\"" + key + "\"";
      const std::size_t key_pos = text.find(pattern);
      if (key_pos == std::string::npos)
      {
        return {};
      }

      const std::size_t value_pos = text.find(':', key_pos);
      if (value_pos == std::string::npos)
      {
        return {};
      }

      const std::size_t array_start = text.find('[', value_pos + 1);
      if (array_start == std::string::npos)
      {
        return {};
      }

      bool in_string = false;
      bool escaped = false;
      int depth = 0;
      for (std::size_t i = array_start; i < text.size(); ++i)
      {
        const char ch = text[i];
        if (escaped)
        {
          escaped = false;
          continue;
        }
        if (ch == '\\')
        {
          escaped = in_string;
          continue;
        }
        if (ch == '"')
        {
          in_string = !in_string;
          continue;
        }
        if (in_string)
        {
          continue;
        }
        if (ch == '[')
        {
          ++depth;
        }
        else if (ch == ']')
        {
          --depth;
          if (depth == 0)
          {
            return text.substr(array_start, i - array_start + 1);
          }
        }
      }

      return {};
    }

    std::vector<std::string> ExtractJsonObjectTexts(const std::string &array_text)
    {
      std::vector<std::string> objects;
      bool in_string = false;
      bool escaped = false;
      int depth = 0;
      std::size_t object_start = std::string::npos;
      for (std::size_t i = 0; i < array_text.size(); ++i)
      {
        const char ch = array_text[i];
        if (escaped)
        {
          escaped = false;
          continue;
        }
        if (ch == '\\')
        {
          escaped = in_string;
          continue;
        }
        if (ch == '"')
        {
          in_string = !in_string;
          continue;
        }
        if (in_string)
        {
          continue;
        }
        if (ch == '{')
        {
          if (depth == 0)
          {
            object_start = i;
          }
          ++depth;
        }
        else if (ch == '}')
        {
          --depth;
          if (depth == 0 && object_start != std::string::npos)
          {
            objects.push_back(array_text.substr(object_start, i - object_start + 1));
            object_start = std::string::npos;
          }
        }
      }

      return objects;
    }

    BuildProfile BuildProfileFromSettings(const EditorSettings &settings)
    {
      BuildProfile profile;
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
      profile.last_executable_path = settings.last_executable_path;
      return profile;
    }

    void ApplyBuildProfileToSettings(const BuildProfile &profile, EditorSettings &settings)
    {
      settings.selected_build_profile = profile.name.empty() ? std::string{"Default"} : profile.name;
      settings.build_mode = profile.build_mode;
      settings.build_socket_host = profile.build_socket_host;
      settings.build_socket_hosts = profile.build_socket_hosts;
      settings.build_socket_port = profile.build_socket_port;
      settings.build_bridge_probe_interval_seconds = std::max(5, std::min(3600, profile.build_bridge_probe_interval_seconds));
      settings.build_command_template = profile.build_command_template;
      settings.project_name = profile.project_name.empty() ? std::string{"TestECS"} : profile.project_name;
      settings.compiler_toolkit = profile.compiler_toolkit;
      settings.build_type = profile.build_type.empty() ? std::string{"Debug"} : profile.build_type;
      settings.target_executable_name = profile.target_executable_name.empty() ? std::string{"TestECS"} : profile.target_executable_name;
      settings.source_directory = profile.source_directory;
      settings.resources_directory = profile.resources_directory;
      settings.build_directory = profile.build_directory;
      settings.header_search_dirs = profile.header_search_dirs;
      settings.library_search_dirs = profile.library_search_dirs;
      settings.export_directory = profile.export_directory;
      settings.last_build_dir = profile.build_directory;
      settings.last_executable_path = profile.last_executable_path;
    }

    void WriteBuildProfileJson(std::ostream &output, const BuildProfile &profile, const std::string &indent)
    {
      output << indent << "{\n";
      output << indent << "  \"name\": \"" << JsonEscape(profile.name.empty() ? std::string{"Default"} : profile.name) << "\",\n";
      output << indent << "  \"buildMode\": \"" << JsonEscape(profile.build_mode) << "\",\n";
      output << indent << "  \"buildSocketHost\": \"" << JsonEscape(profile.build_socket_host) << "\",\n";
      output << indent << "  \"buildSocketHosts\": \"" << JsonEscape(JoinHostList(profile.build_socket_hosts)) << "\",\n";
      output << indent << "  \"buildSocketPort\": " << profile.build_socket_port << ",\n";
      output << indent << "  \"buildBridgeProbeIntervalSeconds\": " << std::max(5, std::min(3600, profile.build_bridge_probe_interval_seconds)) << ",\n";
      output << indent << "  \"buildCommand\": \"" << JsonEscape(profile.build_command_template) << "\",\n";
      output << indent << "  \"projectName\": \"" << JsonEscape(profile.project_name.empty() ? std::string{"TestECS"} : profile.project_name) << "\",\n";
      output << indent << "  \"compilerToolkit\": \"" << JsonEscape(profile.compiler_toolkit) << "\",\n";
      output << indent << "  \"buildType\": \"" << JsonEscape(profile.build_type.empty() ? std::string{"Debug"} : profile.build_type) << "\",\n";
      output << indent << "  \"targetExecutableName\": \"" << JsonEscape(profile.target_executable_name.empty() ? std::string{"TestECS"} : profile.target_executable_name) << "\",\n";
      output << indent << "  \"sourceDirectory\": \"" << JsonEscape(profile.source_directory) << "\",\n";
      output << indent << "  \"resourcesDirectory\": \"" << JsonEscape(profile.resources_directory) << "\",\n";
      output << indent << "  \"buildDirectory\": \"" << JsonEscape(profile.build_directory) << "\",\n";
      output << indent << "  \"headerSearchDirs\": \"" << JsonEscape(profile.header_search_dirs) << "\",\n";
      output << indent << "  \"librarySearchDirs\": \"" << JsonEscape(profile.library_search_dirs) << "\",\n";
      output << indent << "  \"exportDirectory\": \"" << JsonEscape(profile.export_directory) << "\",\n";
      output << indent << "  \"lastExecutablePath\": \"" << JsonEscape(profile.last_executable_path) << "\"\n";
      output << indent << "}";
    }

    BuildProfile ParseBuildProfile(const std::string &profile_text)
    {
      BuildProfile profile;
      profile.name = ExtractJsonString(profile_text, "name");
      if (profile.name.empty())
      {
        profile.name = "Default";
      }
      profile.build_mode = ExtractJsonString(profile_text, "buildMode");
      profile.build_socket_host = ExtractJsonString(profile_text, "buildSocketHost");
      profile.build_socket_hosts = ParseHostList(ExtractJsonString(profile_text, "buildSocketHosts"));
      profile.build_socket_port = ExtractJsonInt(profile_text, "buildSocketPort", profile.build_socket_port);
      profile.build_bridge_probe_interval_seconds = ExtractJsonInt(profile_text, "buildBridgeProbeIntervalSeconds", profile.build_bridge_probe_interval_seconds);
      profile.build_bridge_probe_interval_seconds = std::max(5, std::min(3600, profile.build_bridge_probe_interval_seconds));
      profile.build_command_template = ExtractJsonString(profile_text, "buildCommand");
      profile.project_name = ExtractJsonString(profile_text, "projectName");
      profile.compiler_toolkit = ExtractJsonString(profile_text, "compilerToolkit");
      profile.build_type = ExtractJsonString(profile_text, "buildType");
      profile.target_executable_name = ExtractJsonString(profile_text, "targetExecutableName");
      profile.source_directory = ExtractJsonString(profile_text, "sourceDirectory");
      profile.resources_directory = ExtractJsonString(profile_text, "resourcesDirectory");
      profile.build_directory = ExtractJsonString(profile_text, "buildDirectory");
      profile.header_search_dirs = ExtractJsonString(profile_text, "headerSearchDirs");
      profile.library_search_dirs = ExtractJsonString(profile_text, "librarySearchDirs");
      profile.export_directory = ExtractJsonString(profile_text, "exportDirectory");
      profile.last_executable_path = ExtractJsonString(profile_text, "lastExecutablePath");
      return profile;
    }

    void WriteEditorSettingsJson(std::ostream &output, const EditorSettings &settings)
    {
      output << "{\n";
      output << "  \"build\": {\n";
      output << "    \"buildMode\": \"" << JsonEscape(settings.build_mode) << "\",\n";
      output << "    \"buildSocketHost\": \"" << JsonEscape(settings.build_socket_host) << "\",\n";
      output << "    \"buildSocketHosts\": \"" << JsonEscape(JoinHostList(settings.build_socket_hosts)) << "\",\n";
      output << "    \"buildSocketPort\": " << settings.build_socket_port << ",\n";
      output << "    \"buildBridgeProbeIntervalSeconds\": " << std::max(5, std::min(3600, settings.build_bridge_probe_interval_seconds)) << ",\n";
      output << "    \"buildCommand\": \"" << JsonEscape(settings.build_command_template) << "\",\n";
      output << "    \"compilerToolkit\": \"" << JsonEscape(settings.compiler_toolkit) << "\",\n";
      output << "    \"buildType\": \"" << JsonEscape(settings.build_type.empty() ? std::string{"Debug"} : settings.build_type) << "\",\n";
      output << "    \"targetExecutableName\": \"" << JsonEscape(settings.target_executable_name.empty() ? std::string{"TestECS"} : settings.target_executable_name) << "\",\n";
      output << "    \"sourceDirectory\": \"" << JsonEscape(settings.source_directory) << "\",\n";
      output << "    \"resourcesDirectory\": \"" << JsonEscape(settings.resources_directory) << "\",\n";
      output << "    \"buildDirectory\": \"" << JsonEscape(settings.build_directory) << "\",\n";
      output << "    \"projectMountPath\": \"" << JsonEscape(settings.project_mount_path) << "\",\n";
      output << "    \"headerSearchDirs\": \"" << JsonEscape(settings.header_search_dirs) << "\",\n";
      output << "    \"librarySearchDirs\": \"" << JsonEscape(settings.library_search_dirs) << "\",\n";
      output << "    \"exportDirectory\": \"" << JsonEscape(settings.export_directory) << "\",\n";
      output << "    \"selectedBuildProfile\": \"" << JsonEscape(settings.selected_build_profile.empty() ? std::string{"Default"} : settings.selected_build_profile) << "\",\n";
      output << "    \"buildProfiles\": [\n";
      for (std::size_t i = 0; i < settings.build_profiles.size(); ++i)
      {
        WriteBuildProfileJson(output, settings.build_profiles[i], "      ");
        output << (i + 1 == settings.build_profiles.size() ? "\n" : ",\n");
      }
      output << "    ]\n";
      output << "  },\n";
      output << "  \"project\": {\n";
      output << "    \"projectName\": \"" << JsonEscape(settings.project_name.empty() ? std::string{"TestECS"} : settings.project_name) << "\",\n";
      output << "    \"projectRoot\": \"" << JsonEscape(settings.project_root_path) << "\"\n";
      output << "  },\n";
      output << "  \"ui\": {\n";
      output << "    \"themePreset\": \"" << JsonEscape(settings.theme_preset) << "\",\n";
      output << "    \"uiFontScalePercent\": " << settings.ui_font_scale_percent << ",\n";
      output << "    \"uiRounding\": " << settings.ui_rounding << ",\n";
      output << "    \"uiSpacingPercent\": " << settings.ui_spacing_percent << ",\n";
      output << "    \"uiAccentHex\": \"" << JsonEscape(settings.ui_accent_hex) << "\",\n";
      output << "    \"layoutPresetIndex\": " << settings.layout_preset_index << ",\n";
      output << "    \"editorWindowWidth\": " << settings.editor_window_width << ",\n";
      output << "    \"editorWindowHeight\": " << settings.editor_window_height << "\n";
      output << "  },\n";
      output << "  \"last\": {\n";
      output << "    \"lastCompilerPreset\": \"" << JsonEscape(settings.last_compiler_preset) << "\",\n";
      output << "    \"lastBuildDir\": \"" << JsonEscape(settings.last_build_dir) << "\",\n";
      output << "    \"lastExecutablePath\": \"" << JsonEscape(settings.last_executable_path) << "\"\n";
      output << "  },\n";
      output << "  \"panels\": {\n";
      output << "    \"panelBuildWorkflowOpen\": " << (settings.panel_build_workflow_open ? "true" : "false") << ",\n";
      output << "    \"panelSceneOpen\": " << (settings.panel_scene_open ? "true" : "false") << ",\n";
      output << "    \"panelPreviewOpen\": " << (settings.panel_preview_open ? "true" : "false") << ",\n";
      output << "    \"panelConsoleOpen\": " << (settings.panel_console_open ? "true" : "false") << ",\n";
      output << "    \"panelGameLogOpen\": " << (settings.panel_game_log_open ? "true" : "false") << ",\n";
      output << "    \"panelBuildWorkflowWidth\": " << settings.panel_build_workflow_width << ",\n";
      output << "    \"panelBuildWorkflowHeight\": " << settings.panel_build_workflow_height << ",\n";
      output << "    \"panelPreviewWidth\": " << settings.panel_preview_width << ",\n";
      output << "    \"panelPreviewHeight\": " << settings.panel_preview_height << ",\n";
      output << "    \"panelConsoleWidth\": " << settings.panel_console_width << ",\n";
      output << "    \"panelConsoleHeight\": " << settings.panel_console_height << ",\n";
      output << "    \"panelSceneWidth\": " << settings.panel_scene_width << ",\n";
      output << "    \"panelSceneHeight\": " << settings.panel_scene_height << "\n";
      output << "  }\n";
      output << "}";
    }

    std::string DefaultBuildCommandTemplate()
    {
      return "cd \"{projectRoot}\" && cmake -S \"{projectRoot}\" -B \"{buildDir}\" -DCMAKE_BUILD_TYPE={buildType} {toolchainArg} && cmake --build \"{buildDir}\" --target {target}";
    }

    std::vector<std::string> ParseHostList(const std::string &host_list)
    {
      std::vector<std::string> hosts;

      auto strip_matching_quotes = [](std::string value)
      {
        if (value.size() >= 2)
        {
          const char first = value.front();
          const char last = value.back();
          if ((first == '"' && last == '"') || (first == '\'' && last == '\''))
          {
            value = value.substr(1, value.size() - 2);
          }
        }
        return value;
      };

      auto normalize_host = [&](std::string value)
      {
        value = Trim(value);
        value = strip_matching_quotes(value);
        if (value.empty())
        {
          return std::string{};
        }

        const std::size_t scheme_pos = value.find("://");
        if (scheme_pos != std::string::npos)
        {
          value = value.substr(scheme_pos + 3);
        }

        const std::size_t path_pos = value.find_first_of("/?#");
        if (path_pos != std::string::npos)
        {
          value = value.substr(0, path_pos);
        }

        value = Trim(value);
        value = strip_matching_quotes(value);
        if (value.empty())
        {
          return std::string{};
        }

        if (!value.empty() && value.front() == '[')
        {
          const std::size_t close = value.find(']');
          if (close != std::string::npos)
          {
            value = value.substr(1, close - 1);
            return Trim(value);
          }
        }

        const std::size_t first_colon = value.find(':');
        if (first_colon != std::string::npos)
        {
          const std::size_t last_colon = value.rfind(':');
          if (first_colon == last_colon)
          {
            const std::string maybe_port = value.substr(first_colon + 1);
            const bool is_port = !maybe_port.empty() && std::all_of(maybe_port.begin(), maybe_port.end(), [](unsigned char ch)
                                                                    { return std::isdigit(ch) != 0; });
            if (is_port)
            {
              value = value.substr(0, first_colon);
            }
          }
        }

        return Trim(value);
      };

      std::string current;
      for (const char ch : host_list)
      {
        if (ch == ',' || ch == ';' || ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
        {
          const std::string normalized = normalize_host(current);
          if (!normalized.empty() && std::find(hosts.begin(), hosts.end(), normalized) == hosts.end())
          {
            hosts.push_back(normalized);
          }
          current.clear();
          continue;
        }

        current.push_back(ch);
      }

      const std::string normalized = normalize_host(current);
      if (!normalized.empty() && std::find(hosts.begin(), hosts.end(), normalized) == hosts.end())
      {
        hosts.push_back(normalized);
      }

      return hosts;
    }
  } // namespace

  std::vector<std::string> BuildBridgeHostCandidates(const EditorSettings &settings)
  {
    std::vector<std::string> candidates;
    auto normalize_single_host = [](std::string value)
    {
      value = Trim(value);
      if (value.empty())
      {
        return std::string{};
      }

      const std::size_t scheme_pos = value.find("://");
      if (scheme_pos != std::string::npos)
      {
        value = value.substr(scheme_pos + 3);
      }

      const std::size_t path_pos = value.find_first_of("/?#");
      if (path_pos != std::string::npos)
      {
        value = value.substr(0, path_pos);
      }

      value = Trim(value);
      if (!value.empty() && value.front() == '[')
      {
        const std::size_t close = value.find(']');
        if (close != std::string::npos)
        {
          return Trim(value.substr(1, close - 1));
        }
      }

      const std::size_t first_colon = value.find(':');
      if (first_colon != std::string::npos && first_colon == value.rfind(':'))
      {
        const std::string maybe_port = value.substr(first_colon + 1);
        const bool is_port = !maybe_port.empty() && std::all_of(maybe_port.begin(), maybe_port.end(), [](unsigned char ch)
                                                                { return std::isdigit(ch) != 0; });
        if (is_port)
        {
          value = value.substr(0, first_colon);
        }
      }

      return Trim(value);
    };

    auto append_unique = [&candidates, &normalize_single_host](const std::string &host)
    {
      const std::string normalized = normalize_single_host(host);
      if (!normalized.empty() && std::find(candidates.begin(), candidates.end(), normalized) == candidates.end())
      {
        candidates.push_back(normalized);
      }
    };

    for (const auto &host : settings.build_socket_hosts)
    {
      append_unique(host);
    }

    append_unique(settings.build_socket_host);
    append_unique("127.0.0.1");
    append_unique("localhost");
    append_unique(ResolveWslBridgeHost());

    return candidates;
  }

  bool ShouldUseSocketBuilds(const EditorSettings &settings)
  {
    return !settings.build_socket_host.empty() || Lowercase(settings.build_mode) == "socket";
  }

  EditorSettings LoadEditorSettings(const std::filesystem::path &root)
  {
    EditorSettings settings;

    if (const char *build_mode = std::getenv("MYRIAD_BUILD_MODE"); build_mode != nullptr && *build_mode != '\0')
    {
      settings.build_mode = build_mode;
    }

    if (const char *socket_host = std::getenv("MYRIAD_BUILD_SOCKET_HOST"); socket_host != nullptr && *socket_host != '\0')
    {
      settings.build_socket_host = socket_host;
    }

    if (const char *socket_hosts = std::getenv("MYRIAD_BUILD_SOCKET_HOSTS"); socket_hosts != nullptr && *socket_hosts != '\0')
    {
      settings.build_socket_hosts = ParseHostList(socket_hosts);
    }

    if (const char *socket_port = std::getenv("MYRIAD_BUILD_SOCKET_PORT"); socket_port != nullptr && *socket_port != '\0')
    {
      settings.build_socket_port = ExtractJsonInt(std::string{"{\"port\":"} + socket_port + "}", "port", settings.build_socket_port);
    }

    if (const char *build_command = std::getenv("MYRIAD_BUILD_COMMAND"); build_command != nullptr && *build_command != '\0')
    {
      settings.build_command_template = build_command;
    }

    const std::filesystem::path settings_path = ResolveSettingsReadPath(root);
    if (settings_path.empty())
    {
      return settings;
    }

    std::ifstream input(settings_path);
    if (!input.is_open())
    {
      return settings;
    }

    std::ostringstream contents_stream;
    contents_stream << input.rdbuf();
    const std::string contents = contents_stream.str();
    const std::string file_build_mode = ExtractJsonString(contents, "buildMode");
    if (!file_build_mode.empty())
    {
      settings.build_mode = file_build_mode;
    }

    const std::string file_socket_host = ExtractJsonString(contents, "buildSocketHost");
    if (!file_socket_host.empty())
    {
      settings.build_socket_host = file_socket_host;
    }

    const std::string file_socket_hosts = ExtractJsonString(contents, "buildSocketHosts");
    if (!file_socket_hosts.empty())
    {
      settings.build_socket_hosts = ParseHostList(file_socket_hosts);
    }

    settings.build_socket_port = ExtractJsonInt(contents, "buildSocketPort", settings.build_socket_port);
    settings.build_bridge_probe_interval_seconds = ExtractJsonInt(contents, "buildBridgeProbeIntervalSeconds", settings.build_bridge_probe_interval_seconds);
    settings.build_bridge_probe_interval_seconds = std::max(5, std::min(3600, settings.build_bridge_probe_interval_seconds));
    settings.build_command_template = ExtractJsonString(contents, "buildCommand");
    settings.compiler_toolkit = ExtractJsonString(contents, "compilerToolkit");
    const std::string file_build_type = ExtractJsonString(contents, "buildType");
    if (!file_build_type.empty())
    {
      settings.build_type = file_build_type;
    }
    const std::string file_project_name = ExtractJsonString(contents, "projectName");
    if (!file_project_name.empty())
    {
      settings.project_name = file_project_name;
    }
    const std::string file_target_executable_name = ExtractJsonString(contents, "targetExecutableName");
    if (!file_target_executable_name.empty())
    {
      settings.target_executable_name = file_target_executable_name;
    }
    else
    {
      const std::string legacy_game_project_name = ExtractJsonString(contents, "gameProjectName");
      if (!legacy_game_project_name.empty())
      {
        settings.target_executable_name = legacy_game_project_name;
        if (settings.project_name.empty())
        {
          settings.project_name = legacy_game_project_name;
        }
      }
    }
    settings.source_directory = ExtractJsonString(contents, "sourceDirectory");
    settings.resources_directory = ExtractJsonString(contents, "resourcesDirectory");
    settings.build_directory = ExtractJsonString(contents, "buildDirectory");
    settings.project_root_path = ExtractJsonString(contents, "projectRoot");
    settings.project_mount_path = ExtractJsonString(contents, "projectMountPath");
    settings.header_search_dirs = ExtractJsonString(contents, "headerSearchDirs");
    settings.library_search_dirs = ExtractJsonString(contents, "librarySearchDirs");
    settings.export_directory = ExtractJsonString(contents, "exportDirectory");
    settings.selected_build_profile = ExtractJsonString(contents, "selectedBuildProfile");
    const std::string file_theme_preset = ExtractJsonString(contents, "themePreset");
    if (!file_theme_preset.empty())
    {
      settings.theme_preset = file_theme_preset;
    }

    settings.ui_font_scale_percent = ExtractJsonInt(contents, "uiFontScalePercent", settings.ui_font_scale_percent);
    settings.ui_rounding = ExtractJsonInt(contents, "uiRounding", settings.ui_rounding);
    settings.ui_spacing_percent = ExtractJsonInt(contents, "uiSpacingPercent", settings.ui_spacing_percent);
    const std::string file_ui_accent_hex = ExtractJsonString(contents, "uiAccentHex");
    if (!file_ui_accent_hex.empty())
    {
      settings.ui_accent_hex = file_ui_accent_hex;
    }

    settings.last_compiler_preset = ExtractJsonString(contents, "lastCompilerPreset");
    settings.last_build_dir = ExtractJsonString(contents, "lastBuildDir");
    if (settings.build_directory.empty())
    {
      settings.build_directory = settings.last_build_dir;
    }
    settings.last_executable_path = ExtractJsonString(contents, "lastExecutablePath");

    const std::string profiles_array = ExtractJsonArrayText(contents, "buildProfiles");
    for (const auto &profile_text : ExtractJsonObjectTexts(profiles_array))
    {
      BuildProfile profile = ParseBuildProfile(profile_text);
      if (!profile.name.empty())
      {
        settings.build_profiles.push_back(profile);
      }
    }
    if (settings.build_profiles.empty())
    {
      if (settings.selected_build_profile.empty())
      {
        settings.selected_build_profile = "Default";
      }
      settings.build_profiles.push_back(BuildProfileFromSettings(settings));
    }
    if (settings.selected_build_profile.empty())
    {
      settings.selected_build_profile = settings.build_profiles.front().name;
    }
    auto selected_profile = std::find_if(settings.build_profiles.begin(), settings.build_profiles.end(), [&settings](const BuildProfile &profile)
                                         { return profile.name == settings.selected_build_profile; });
    if (selected_profile == settings.build_profiles.end())
    {
      selected_profile = settings.build_profiles.begin();
    }
    ApplyBuildProfileToSettings(*selected_profile, settings);
    settings.layout_preset_index = ExtractJsonInt(contents, "layoutPresetIndex", settings.layout_preset_index);

    settings.editor_window_width = ExtractJsonInt(contents, "editorWindowWidth", settings.editor_window_width);
    settings.editor_window_height = ExtractJsonInt(contents, "editorWindowHeight", settings.editor_window_height);
    settings.panel_build_workflow_open = ExtractJsonBool(contents, "panelBuildWorkflowOpen", settings.panel_build_workflow_open);
    settings.panel_scene_open = ExtractJsonBool(contents, "panelSceneOpen", settings.panel_scene_open);
    settings.panel_preview_open = ExtractJsonBool(contents, "panelPreviewOpen", settings.panel_preview_open);
    settings.panel_console_open = ExtractJsonBool(contents, "panelConsoleOpen", settings.panel_console_open);
    settings.panel_game_log_open = ExtractJsonBool(contents, "panelGameLogOpen", settings.panel_game_log_open);

    settings.panel_build_workflow_width = ExtractJsonInt(contents, "panelBuildWorkflowWidth", settings.panel_build_workflow_width);
    settings.panel_build_workflow_height = ExtractJsonInt(contents, "panelBuildWorkflowHeight", settings.panel_build_workflow_height);
    settings.panel_preview_width = ExtractJsonInt(contents, "panelPreviewWidth", settings.panel_preview_width);
    settings.panel_preview_height = ExtractJsonInt(contents, "panelPreviewHeight", settings.panel_preview_height);
    settings.panel_console_width = ExtractJsonInt(contents, "panelConsoleWidth", settings.panel_console_width);
    settings.panel_console_height = ExtractJsonInt(contents, "panelConsoleHeight", settings.panel_console_height);
    settings.panel_scene_width = ExtractJsonInt(contents, "panelSceneWidth", settings.panel_scene_width);
    settings.panel_scene_height = ExtractJsonInt(contents, "panelSceneHeight", settings.panel_scene_height);

    return settings;
  }

  std::filesystem::path GetEditorSettingsReadPath(const std::filesystem::path &root)
  {
    return ResolveSettingsReadPath(root);
  }

  std::filesystem::path GetEditorSettingsWritePath(const std::filesystem::path &root)
  {
    return ResolveSettingsWritePath(root);
  }

  std::vector<std::string> GetEditorSettingsDiscoveryLog(const std::filesystem::path &root)
  {
    std::vector<std::string> lines;
    const bool installed_run = IsInstalledEditorRun(root);
    const bool root_is_repo = !root.empty() && std::filesystem::exists(root / "Editor") && std::filesystem::exists(root / "Engine");
    const std::filesystem::path override_settings = ResolveEditorConfigOverridePath();
    const std::filesystem::path resource_settings = ResolveEditorResourceConfigPath(root);
    const std::filesystem::path project_settings = root.empty() ? std::filesystem::path{} : root / ".myriad_editor" / "editor.json";
    const std::filesystem::path legacy_project_settings = root.empty() ? std::filesystem::path{} : root / ".myriad-editor.json";

    lines.push_back("Settings discovery root: " + (root.empty() ? std::string{"<none>"} : root.string()));
    lines.push_back(BoolStatusLine("Settings discovery installed run", installed_run));
    lines.push_back(BoolStatusLine("Settings discovery root is Myriad repo", root_is_repo));
    lines.push_back(PathStatusLine("Settings override path", override_settings));
    lines.push_back(PathStatusLine("Settings project path", project_settings));
    lines.push_back(PathStatusLine("Settings legacy project path", legacy_project_settings));
    lines.push_back(PathStatusLine("Settings resource path", resource_settings));
    lines.push_back(PathStatusLine("Settings effective read path", ResolveSettingsReadPath(root)));
    lines.push_back(PathStatusLine("Settings effective write path", ResolveSettingsWritePath(root)));
    return lines;
  }

  bool SaveEditorSettings(const std::filesystem::path &root, const EditorSettings &settings)
  {
    const std::filesystem::path settings_path = ResolveSettingsWritePath(root);
    if (settings_path.empty())
    {
      return false;
    }

    const std::filesystem::path settings_parent = settings_path.parent_path();
    if (!settings_parent.empty())
    {
      std::error_code error_code;
      std::filesystem::create_directories(settings_parent, error_code);
    }

    std::ofstream output(settings_path, std::ios::trunc);
    if (!output.is_open())
    {
      return false;
    }

    EditorSettings normalized_settings = settings;
    normalized_settings.build_directory = ProjectRelativePathText(root, normalized_settings.build_directory);
    normalized_settings.last_build_dir = ProjectRelativePathText(root, normalized_settings.last_build_dir);
    if (normalized_settings.selected_build_profile.empty())
    {
      normalized_settings.selected_build_profile = "Default";
    }
    if (normalized_settings.build_profiles.empty())
    {
      normalized_settings.build_profiles.push_back(BuildProfileFromSettings(normalized_settings));
    }
    auto selected_profile = std::find_if(normalized_settings.build_profiles.begin(), normalized_settings.build_profiles.end(), [&normalized_settings](const BuildProfile &profile)
                                         { return profile.name == normalized_settings.selected_build_profile; });
    if (selected_profile == normalized_settings.build_profiles.end())
    {
      normalized_settings.build_profiles.push_back(BuildProfileFromSettings(normalized_settings));
      selected_profile = std::prev(normalized_settings.build_profiles.end());
    }
    *selected_profile = BuildProfileFromSettings(normalized_settings);
    for (auto &profile : normalized_settings.build_profiles)
    {
      profile.build_directory = ProjectRelativePathText(root, profile.build_directory);
    }

    WriteEditorSettingsJson(output, normalized_settings);
    output << "\n";
    return output.good();
  }

  std::string ResolveBuildCommandTemplate(const EditorSettings &settings)
  {
    if (!settings.build_command_template.empty())
    {
      return settings.build_command_template;
    }

    return DefaultBuildCommandTemplate();
  }

  std::string BuildDirForBridge(const std::filesystem::path &build_dir, const std::filesystem::path &project_root)
  {
    std::error_code error_code;
    const std::filesystem::path relative_build_dir = std::filesystem::relative(build_dir, project_root, error_code);
    if (error_code || relative_build_dir.empty())
    {
      return {};
    }

    const std::string relative_text = relative_build_dir.generic_string();
    if (relative_text.rfind("..", 0) == 0)
    {
      return {};
    }

    return relative_text;
  }

  std::string CreateBuildBridgeRequest(const CompilerPreset &preset,
                                       const std::string &build_dir_relative,
                                       const std::string &project_mount_path,
                                       const std::string &build_type,
                                       const std::string &target_executable_name,
                                       const std::string &source_directory,
                                       const std::string &resources_directory,
                                       const std::string &header_search_dirs,
                                       const std::string &library_search_dirs,
                                       const std::string &export_directory)
  {
    return std::string{"{\"compilerToolkit\":\""} + JsonEscape(preset.name) +
           "\",\"buildDir\":\"" + JsonEscape(build_dir_relative) +
           "\",\"projectPath\":\"" + JsonEscape(project_mount_path) +
           "\",\"buildType\":\"" + JsonEscape(build_type.empty() ? std::string{"Debug"} : build_type) +
           "\",\"projectName\":\"" + JsonEscape(target_executable_name.empty() ? std::string{"TestECS"} : target_executable_name) +
           "\",\"sourceDirectory\":\"" + JsonEscape(source_directory) +
           "\",\"resourcesDirectory\":\"" + JsonEscape(resources_directory) +
           "\",\"headerSearchDirs\":\"" + JsonEscape(header_search_dirs) +
           "\",\"librarySearchDirs\":\"" + JsonEscape(library_search_dirs) +
           "\",\"exportDirectory\":\"" + JsonEscape(export_directory) +
           "\",\"target\":\"" + JsonEscape(target_executable_name.empty() ? std::string{"TestECS"} : target_executable_name) + "\"}\n";
  }

  std::string CreateBuildOptionsRequest(const std::string &project_mount_path, const std::string &compiler_toolkit, const std::string &build_type)
  {
    return std::string{"{\"action\":\"listBuildOptions\",\"projectPath\":\""} + JsonEscape(project_mount_path) +
           "\",\"compilerToolkit\":\"" + JsonEscape(compiler_toolkit) +
           "\",\"buildType\":\"" + JsonEscape(build_type.empty() ? std::string{"Debug"} : build_type) + "\"}\n";
  }

  std::string CreateProjectListRequest(const std::string &relative_path)
  {
    return std::string{"{\"action\":\"listProjects\",\"path\":\""} + JsonEscape(relative_path) + "\"}\n";
  }

  std::string CreateProjectOpenRequest(const std::string &relative_path)
  {
    return std::string{"{\"action\":\"openProject\",\"projectPath\":\""} + JsonEscape(relative_path) + "\"}\n";
  }

  std::string CreateProjectCreateRequest(const std::string &parent_path, const std::string &project_name)
  {
    return std::string{"{\"action\":\"createProject\",\"parent\":\""} + JsonEscape(parent_path) +
           "\",\"name\":\"" + JsonEscape(project_name) + "\"}\n";
  }

  std::string CreateDirectoryListRequest(const std::string &relative_path)
  {
    return std::string{"{\"action\":\"listDirectories\",\"path\":\""} + JsonEscape(relative_path) + "\"}\n";
  }

  std::string CreateDirectoryCreateRequest(const std::string &parent_path, const std::string &directory_name)
  {
    return std::string{"{\"action\":\"createDirectory\",\"parent\":\""} + JsonEscape(parent_path) +
           "\",\"name\":\"" + JsonEscape(directory_name) + "\"}\n";
  }

  std::string CreateProjectSettingsSaveRequest(const EditorSettings &settings)
  {
    std::ostringstream settings_json;
    WriteEditorSettingsJson(settings_json, settings);
    std::string settings_text = settings_json.str();
    ReplaceAll(settings_text, "\r", "");
    ReplaceAll(settings_text, "\n", "");
    return std::string{"{\"action\":\"saveProjectSettings\",\"projectPath\":\""} + JsonEscape(settings.project_mount_path) +
           "\",\"settings\":" + settings_text + "}\n";
  }

  std::string CreateBuildBridgeStatusRequest(const std::string &project_mount_path, const std::string &target_executable_name, const std::string &source_directory)
  {
    return std::string{"{\"action\":\"status\",\"projectPath\":\""} + JsonEscape(project_mount_path) +
           "\",\"projectName\":\"" + JsonEscape(target_executable_name.empty() ? std::string{"TestECS"} : target_executable_name) +
           "\",\"sourceDirectory\":\"" + JsonEscape(source_directory) + "\"}\n";
  }
} // namespace Editor
