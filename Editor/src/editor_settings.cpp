#include "editor_settings.h"

#include "json_utils.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace Editor
{
  namespace
  {
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
  } // namespace

  std::string DefaultBuildCommandTemplate()
  {
    return "cd \"{projectRoot}\" && cmake -S \"{projectRoot}\" -B \"{buildDir}\" -DCMAKE_BUILD_TYPE=Debug {toolchainArg} && cmake --build \"{buildDir}\" --target {target}";
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

    const std::filesystem::path settings_path = root / ".myriad-editor.json";
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
    settings.last_executable_path = ExtractJsonString(contents, "lastExecutablePath");
    settings.layout_preset_index = ExtractJsonInt(contents, "layoutPresetIndex", settings.layout_preset_index);

    settings.editor_window_width = ExtractJsonInt(contents, "editorWindowWidth", settings.editor_window_width);
    settings.editor_window_height = ExtractJsonInt(contents, "editorWindowHeight", settings.editor_window_height);
    settings.panel_build_workflow_open = ExtractJsonBool(contents, "panelBuildWorkflowOpen", settings.panel_build_workflow_open);
    settings.panel_scene_open = ExtractJsonBool(contents, "panelSceneOpen", settings.panel_scene_open);
    settings.panel_preview_open = ExtractJsonBool(contents, "panelPreviewOpen", settings.panel_preview_open);
    settings.panel_console_open = ExtractJsonBool(contents, "panelConsoleOpen", settings.panel_console_open);
    settings.panel_game_log_open = ExtractJsonBool(contents, "panelGameLogOpen", settings.panel_game_log_open);
    settings.panel_preferences_open = ExtractJsonBool(contents, "panelPreferencesOpen", settings.panel_preferences_open);

    settings.panel_build_workflow_width = ExtractJsonInt(contents, "panelBuildWorkflowWidth", settings.panel_build_workflow_width);
    settings.panel_build_workflow_height = ExtractJsonInt(contents, "panelBuildWorkflowHeight", settings.panel_build_workflow_height);
    settings.panel_preview_width = ExtractJsonInt(contents, "panelPreviewWidth", settings.panel_preview_width);
    settings.panel_preview_height = ExtractJsonInt(contents, "panelPreviewHeight", settings.panel_preview_height);
    settings.panel_console_width = ExtractJsonInt(contents, "panelConsoleWidth", settings.panel_console_width);
    settings.panel_console_height = ExtractJsonInt(contents, "panelConsoleHeight", settings.panel_console_height);
    settings.panel_workspace_width = ExtractJsonInt(contents, "panelWorkspaceWidth", settings.panel_workspace_width);
    settings.panel_workspace_height = ExtractJsonInt(contents, "panelWorkspaceHeight", settings.panel_workspace_height);
    settings.panel_preferences_width = ExtractJsonInt(contents, "panelPreferencesWidth", settings.panel_preferences_width);
    settings.panel_preferences_height = ExtractJsonInt(contents, "panelPreferencesHeight", settings.panel_preferences_height);
    settings.panel_scene_width = ExtractJsonInt(contents, "panelSceneWidth", settings.panel_scene_width);
    settings.panel_scene_height = ExtractJsonInt(contents, "panelSceneHeight", settings.panel_scene_height);

    return settings;
  }

  bool SaveEditorSettings(const std::filesystem::path &root, const EditorSettings &settings)
  {
    const std::filesystem::path settings_path = root / ".myriad-editor.json";
    std::ofstream output(settings_path, std::ios::trunc);
    if (!output.is_open())
    {
      return false;
    }

    output << "{\n";
    output << "  \"buildMode\": \"" << JsonEscape(settings.build_mode) << "\",\n";
    output << "  \"buildSocketHost\": \"" << JsonEscape(settings.build_socket_host) << "\",\n";
    output << "  \"buildSocketHosts\": \"" << JsonEscape(JoinHostList(settings.build_socket_hosts)) << "\",\n";
    output << "  \"buildSocketPort\": " << settings.build_socket_port << ",\n";
    output << "  \"buildBridgeProbeIntervalSeconds\": " << std::max(5, std::min(3600, settings.build_bridge_probe_interval_seconds)) << ",\n";
    output << "  \"buildCommand\": \"" << JsonEscape(settings.build_command_template) << "\",\n";
    output << "  \"themePreset\": \"" << JsonEscape(settings.theme_preset) << "\",\n";
    output << "  \"uiFontScalePercent\": " << settings.ui_font_scale_percent << ",\n";
    output << "  \"uiRounding\": " << settings.ui_rounding << ",\n";
    output << "  \"uiSpacingPercent\": " << settings.ui_spacing_percent << ",\n";
    output << "  \"uiAccentHex\": \"" << JsonEscape(settings.ui_accent_hex) << "\",\n";
    output << "  \"lastCompilerPreset\": \"" << JsonEscape(settings.last_compiler_preset) << "\",\n";
    output << "  \"lastBuildDir\": \"" << JsonEscape(settings.last_build_dir) << "\",\n";
    output << "  \"lastExecutablePath\": \"" << JsonEscape(settings.last_executable_path) << "\",\n";
    output << "  \"layoutPresetIndex\": " << settings.layout_preset_index << ",\n";
    output << "  \"editorWindowWidth\": " << settings.editor_window_width << ",\n";
    output << "  \"editorWindowHeight\": " << settings.editor_window_height << ",\n";
    output << "  \"panelBuildWorkflowOpen\": " << (settings.panel_build_workflow_open ? "true" : "false") << ",\n";
    output << "  \"panelSceneOpen\": " << (settings.panel_scene_open ? "true" : "false") << ",\n";
    output << "  \"panelPreviewOpen\": " << (settings.panel_preview_open ? "true" : "false") << ",\n";
    output << "  \"panelConsoleOpen\": " << (settings.panel_console_open ? "true" : "false") << ",\n";
    output << "  \"panelGameLogOpen\": " << (settings.panel_game_log_open ? "true" : "false") << ",\n";
    output << "  \"panelPreferencesOpen\": " << (settings.panel_preferences_open ? "true" : "false") << ",\n";
    output << "  \"panelBuildWorkflowWidth\": " << settings.panel_build_workflow_width << ",\n";
    output << "  \"panelBuildWorkflowHeight\": " << settings.panel_build_workflow_height << ",\n";
    output << "  \"panelPreviewWidth\": " << settings.panel_preview_width << ",\n";
    output << "  \"panelPreviewHeight\": " << settings.panel_preview_height << ",\n";
    output << "  \"panelConsoleWidth\": " << settings.panel_console_width << ",\n";
    output << "  \"panelConsoleHeight\": " << settings.panel_console_height << ",\n";
    output << "  \"panelWorkspaceWidth\": " << settings.panel_workspace_width << ",\n";
    output << "  \"panelWorkspaceHeight\": " << settings.panel_workspace_height << ",\n";
    output << "  \"panelPreferencesWidth\": " << settings.panel_preferences_width << ",\n";
    output << "  \"panelPreferencesHeight\": " << settings.panel_preferences_height << ",\n";
    output << "  \"panelSceneWidth\": " << settings.panel_scene_width << ",\n";
    output << "  \"panelSceneHeight\": " << settings.panel_scene_height << "\n";
    output << "}\n";
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

  std::string CreateBuildBridgeRequest(const CompilerPreset &preset, const std::string &build_dir_relative)
  {
    return std::string{"{\"preset\":\""} + JsonEscape(preset.name) +
           "\",\"buildDir\":\"" + JsonEscape(build_dir_relative) +
           "\",\"target\":\"TestECS\"}\n";
  }

  std::string CreateBuildBridgeStatusRequest()
  {
    return "{\"action\":\"status\"}\n";
  }
} // namespace Editor
