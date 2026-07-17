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
    const std::filesystem::path themes_path = project_root / "Editor" / "themes.json";
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
    const std::filesystem::path layouts_path = project_root / "Editor" / "layouts.json";
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

  std::filesystem::path FindHostedLibraryPath(const std::filesystem::path &build_dir)
  {
    const std::vector<std::filesystem::path> candidates = {
        build_dir / "Examples" / "TestECS" / "libTestECS.so",
        build_dir / "Examples" / "TestECS" / "TestECS.dll",
        build_dir / "Examples" / "TestECS" / "libTestECS.dylib",
    };

    for (const auto &candidate : candidates)
    {
      if (std::filesystem::exists(candidate))
      {
        return candidate;
      }
    }

    const std::filesystem::path search_root = build_dir / "Examples" / "TestECS";
    if (std::filesystem::exists(search_root))
    {
      for (const auto &entry : std::filesystem::recursive_directory_iterator(search_root))
      {
        if (!entry.is_regular_file())
        {
          continue;
        }

        const std::string filename = entry.path().filename().string();
        if (filename == "libTestECS.so" || filename == "TestECS.dll" || filename == "libTestECS.dylib")
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

    struct AnsiStyle
    {
      ImVec4 fg = ImGui::GetStyleColorVec4(ImGuiCol_Text);
      bool has_bg = false;
      ImVec4 bg = ImVec4(0.0f, 0.0f, 0.0f, 0.45f);
    };

    std::vector<Segment> segments;
    std::string buffer;
    AnsiStyle style{};
    const ImVec4 default_fg = ImGui::GetStyleColorVec4(ImGuiCol_Text);

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
  void ApplyDockLayoutPreset(const ImGuiID dockspace_id, const Editor::LayoutPreset &preset)
  {
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    if (viewport == nullptr)
    {
      return;
    }

    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

    ImGuiID dock_main = dockspace_id;
    ImGuiID dock_left = 0;
    ImGuiID dock_right = 0;
    ImGuiID dock_bottom = 0;
    ImGuiID dock_center = dock_main;

    const float left_ratio = static_cast<float>(std::max(5, std::min(60, preset.left_percent))) / 100.0f;
    const float right_ratio = static_cast<float>(std::max(5, std::min(60, preset.right_percent))) / 100.0f;
    const float bottom_ratio = static_cast<float>(std::max(10, std::min(70, preset.bottom_percent))) / 100.0f;
    const std::string mode = Editor::Lowercase(preset.mode);

    if (mode == "preview")
    {
      dock_right = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Right, right_ratio, nullptr, &dock_main);
      dock_left = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Left, left_ratio, nullptr, &dock_main);
      dock_bottom = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Down, bottom_ratio, nullptr, &dock_main);
      dock_center = dock_main;
    }
    else if (mode == "build")
    {
      dock_left = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Left, left_ratio, nullptr, &dock_main);
      dock_bottom = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Down, bottom_ratio, nullptr, &dock_main);
      dock_right = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Right, right_ratio, nullptr, &dock_main);
      dock_center = dock_main;
    }
    else
    {
      dock_left = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Left, left_ratio, nullptr, &dock_main);
      dock_right = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Right, right_ratio, nullptr, &dock_main);
      dock_bottom = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Down, bottom_ratio, nullptr, &dock_main);
      dock_center = dock_main;
    }

    ImGui::DockBuilderDockWindow("Build Workflow", dock_left != 0 ? dock_left : dock_center);
    ImGui::DockBuilderDockWindow("Editor Preferences", dock_right != 0 ? dock_right : dock_center);
    ImGui::DockBuilderDockWindow("Console", dock_bottom != 0 ? dock_bottom : dock_center);
    ImGui::DockBuilderDockWindow("Game Log", dock_bottom != 0 ? dock_bottom : dock_center);
    ImGui::DockBuilderDockWindow("Scene", dock_center);
    ImGui::DockBuilderDockWindow("Preview", dock_center);
    ImGui::DockBuilderFinish(dockspace_id);
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
}

void MyriadEditor::Start()
{
  MyrGameApplication::Start();
  rlImGuiSetup(true);
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
#ifdef IMGUI_HAS_DOCK
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#endif
  project_root_ = Editor::FindProjectRoot();
  compiler_presets_ = Editor::LoadCompilerPresets(project_root_);
  theme_presets_ = LoadThemePresets(project_root_);
  layout_presets_ = LoadLayoutPresets(project_root_);
  editor_settings_ = Editor::LoadEditorSettings(project_root_);
  build_command_template_ = Editor::ResolveBuildCommandTemplate(editor_settings_);
  if (compiler_presets_.empty())
  {
    compiler_presets_.push_back({"Default", {}});
  }

  selected_preset_index_ = 0;
  if (!editor_settings_.last_compiler_preset.empty())
  {
    for (int i = 0; i < static_cast<int>(compiler_presets_.size()); ++i)
    {
      if (compiler_presets_[i].name == editor_settings_.last_compiler_preset)
      {
        selected_preset_index_ = i;
        break;
      }
    }
  }

  build_dir_input_ = editor_settings_.last_build_dir;
  executable_input_ = editor_settings_.last_executable_path;
  selected_theme_preset_index_ = ThemePresetIndexFromName(editor_settings_.theme_preset, theme_presets_);

  ui_font_scale_ = static_cast<float>(editor_settings_.ui_font_scale_percent) / 100.0f;
  ui_rounding_ = static_cast<float>(editor_settings_.ui_rounding);
  ui_spacing_density_ = static_cast<float>(editor_settings_.ui_spacing_percent) / 100.0f;
  if (!ParseHexColor(editor_settings_.ui_accent_hex, ui_accent_color_))
  {
    ui_accent_color_ = ImVec4(0.30f, 0.54f, 0.81f, 1.0f);
  }

  ApplyThemePresetByIndex(selected_theme_preset_index_);
  ApplyThemeCustomizations();
#ifdef IMGUI_HAS_DOCK
  if (layout_presets_.empty())
  {
    layout_presets_ = DefaultLayoutPresets();
  }
  selected_layout_preset_index_ = std::max(0, std::min(editor_settings_.layout_preset_index, static_cast<int>(layout_presets_.size()) - 1));
#else
  selected_layout_preset_index_ = 0;
#endif
  show_build_workflow_window_ = editor_settings_.panel_build_workflow_open;
  show_scene_window_ = editor_settings_.panel_scene_open;
  show_preview_window_ = editor_settings_.panel_preview_open;
  show_console_window_ = editor_settings_.panel_console_open;
  show_game_log_window_ = editor_settings_.panel_game_log_open;
  show_editor_preferences_window_ = editor_settings_.panel_preferences_open;
  dock_layout_apply_requested_ = false;
  RefreshPaths(false);
  last_selected_preset_index_ = selected_preset_index_;

  if (editor_settings_.editor_window_width > 0 && editor_settings_.editor_window_height > 0)
  {
    SetWindowSize(editor_settings_.editor_window_width, editor_settings_.editor_window_height);
  }

  editor_settings_.last_compiler_preset = compiler_presets_[selected_preset_index_].name;
  editor_settings_.last_build_dir = build_dir_input_;
  editor_settings_.last_executable_path = executable_input_;
  editor_settings_.layout_preset_index = selected_layout_preset_index_;
  status_ = "Ready. Choose a compiler preset, then build and run the TestECS example.";
}

void MyriadEditor::Render()
{
  ClearBackground({0, 0, 0, 255});

  bool settings_dirty = false;
  auto mark_settings_dirty = [&settings_dirty]()
  {
    settings_dirty = true;
  };

  const int current_window_width = GetScreenWidth();
  const int current_window_height = GetScreenHeight();
  if (editor_settings_.editor_window_width != current_window_width || editor_settings_.editor_window_height != current_window_height)
  {
    editor_settings_.editor_window_width = current_window_width;
    editor_settings_.editor_window_height = current_window_height;
    mark_settings_dirty();
  }

  if (preview_stop_requested_)
  {
    StopPreviewGame();
    EndPreviewLogCapture();
    preview_stop_requested_ = false;
  }
  else if (preview_texture_cleanup_requested_)
  {
    preview_texture_ = {};
    preview_texture_ready_ = false;
    preview_texture_cleanup_requested_ = false;
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
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, -status_bar_height), ImGuiDockNodeFlags_None);

    if (dock_layout_apply_requested_)
    {
      show_build_workflow_window_ = true;
      show_scene_window_ = true;
      show_preview_window_ = true;
      show_console_window_ = true;
      show_game_log_window_ = true;
      show_editor_preferences_window_ = true;
      if (layout_presets_.empty())
      {
        layout_presets_ = DefaultLayoutPresets();
      }
      const int safe_layout_index = std::max(0, std::min(selected_layout_preset_index_, static_cast<int>(layout_presets_.size()) - 1));
      ApplyDockLayoutPreset(dockspace_id, layout_presets_[safe_layout_index]);
      dock_layout_apply_requested_ = false;
    }

    ImGui::SetCursorPos(ImVec2(0.0f, ImGui::GetWindowHeight() - status_bar_height));
    ImGui::Separator();
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(18.0f, 4.0f));
    ImGui::BeginChild("BottomStatusBar", ImVec2(0.0f, status_bar_height - 2.0f), false, ImGuiWindowFlags_NoScrollbar);
    const int safe_preset_index = std::max(0, std::min(selected_preset_index_, static_cast<int>(compiler_presets_.size()) - 1));
    const std::string selected_toolchain_label = compiler_presets_.empty() ? std::string{"Default"} : compiler_presets_[safe_preset_index].name;
    if (Editor::ShouldUseSocketBuilds(editor_settings_))
    {
      ImGui::TextUnformatted("Build Bridge:");
      ImGui::SameLine();
      const ImVec4 bridge_color = build_bridge_connected_ ? ImVec4(0.20f, 0.78f, 0.34f, 1.0f) : ImVec4(0.88f, 0.34f, 0.25f, 1.0f);
      ImGui::TextColored(bridge_color, "%s", build_bridge_connected_ ? "Connected" : "Disconnected");

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
      std::strncpy(bridge_host_buffer, editor_settings_.build_socket_host.c_str(), sizeof(bridge_host_buffer) - 1);
      bridge_host_buffer[sizeof(bridge_host_buffer) - 1] = '\0';
      if (ImGui::InputText("##BridgeHost", bridge_host_buffer, sizeof(bridge_host_buffer)))
      {
        const std::string normalized_host = Editor::Trim(std::string(bridge_host_buffer));
        editor_settings_.build_socket_host = normalized_host;
        if (!normalized_host.empty() && std::find(editor_settings_.build_socket_hosts.begin(), editor_settings_.build_socket_hosts.end(), normalized_host) == editor_settings_.build_socket_hosts.end())
        {
          editor_settings_.build_socket_hosts.push_back(normalized_host);
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
      std::snprintf(bridge_port_buffer, sizeof(bridge_port_buffer), "%d", editor_settings_.build_socket_port);
      if (ImGui::InputText("##BridgePort", bridge_port_buffer, sizeof(bridge_port_buffer), ImGuiInputTextFlags_CharsDecimal))
      {
        const std::string port_text = Editor::Trim(std::string(bridge_port_buffer));
        if (!port_text.empty())
        {
          const int parsed_port = std::atoi(port_text.c_str());
          const int clamped_port = std::max(1, std::min(65535, parsed_port));
          if (clamped_port != editor_settings_.build_socket_port)
          {
            editor_settings_.build_socket_port = clamped_port;
            mark_settings_dirty();
          }
        }
      }

      if (bridge_build_in_progress_)
      {
        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.95f, 0.80f, 0.25f, 1.0f), "%s Building", CurrentBuildSpinnerFrame());
        if (bridge_build_progress_percent_ >= 0)
        {
          ImGui::SameLine();
          ImGui::TextDisabled("|");
          ImGui::SameLine();
          const float progress_fraction = static_cast<float>(bridge_build_progress_percent_) / 100.0f;
          const std::string progress_overlay = std::to_string(bridge_build_progress_percent_) + "%";
          ImGui::ProgressBar(progress_fraction, ImVec2(130.0f, 0.0f), progress_overlay.c_str());
        }
      }
      else if (build_succeeded_)
      {
        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.35f, 0.85f, 0.45f, 1.0f), "Build OK");
      }

      if (bridge_rebuild_needed_)
      {
        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.95f, 0.75f, 0.25f, 1.0f), "Rebuild needed (%d)", bridge_changed_file_count_);
      }

      if (!build_bridge_status_text_.empty())
      {
        ImGui::SameLine();
        ImGui::TextDisabled("| %s", build_bridge_status_text_.c_str());
      }

      if (build_bridge_warning_active_)
      {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.95f, 0.45f, 0.30f, 1.0f), "Warning");
      }

      ImGui::SameLine();
      ImGui::TextDisabled("|");
      ImGui::SameLine();
      if (ImGui::SmallButton(("Toolchain: " + selected_toolchain_label).c_str()))
      {
        show_build_workflow_window_ = true;
      }
    }
    else
    {
      ImGui::TextDisabled("Build bridge inactive (local build mode).");
      ImGui::SameLine();
      ImGui::TextDisabled("|");
      ImGui::SameLine();
      if (ImGui::SmallButton(("Toolchain: " + selected_toolchain_label).c_str()))
      {
        show_build_workflow_window_ = true;
      }
      if (!status_.empty())
      {
        ImGui::SameLine();
        ImGui::TextDisabled("| %s", status_.c_str());
      }
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();

    ImGui::End();
  }
#endif

  if (ImGui::BeginMainMenuBar())
  {
    if (ImGui::BeginMenu("Window"))
    {
      ImGui::MenuItem("Build Workflow", nullptr, &show_build_workflow_window_);
      ImGui::MenuItem("Editor Preferences", nullptr, &show_editor_preferences_window_);
      ImGui::MenuItem("Scene", nullptr, &show_scene_window_);
      ImGui::MenuItem("Preview", nullptr, &show_preview_window_);
      ImGui::MenuItem("Console", nullptr, &show_console_window_);
      ImGui::MenuItem("Game Log", nullptr, &show_game_log_window_);
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Run"))
    {
      const bool can_start_build = !bridge_build_in_progress_;
      if (ImGui::MenuItem("Build TestECS", nullptr, false, can_start_build))
      {
        run_after_build_request_ = false;
        BuildTestECS();
      }
      if (ImGui::MenuItem("Build && Run", nullptr, false, can_start_build))
      {
        run_after_build_request_ = true;
        BuildTestECS();
        if (!Editor::ShouldUseSocketBuilds(editor_settings_) && build_succeeded_)
        {
          run_after_build_request_ = false;
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

  if (selected_preset_index_ != last_selected_preset_index_)
  {
    last_selected_preset_index_ = selected_preset_index_;
    RefreshPaths(true);
    editor_settings_.last_compiler_preset = compiler_presets_[selected_preset_index_].name;
    editor_settings_.last_build_dir = build_dir_input_;
    editor_settings_.last_executable_path = executable_input_;
    mark_settings_dirty();
  }

  if (editor_settings_.panel_build_workflow_open != show_build_workflow_window_ ||
      editor_settings_.panel_scene_open != show_scene_window_ ||
      editor_settings_.panel_preview_open != show_preview_window_ ||
      editor_settings_.panel_console_open != show_console_window_ ||
      editor_settings_.panel_game_log_open != show_game_log_window_ ||
      editor_settings_.panel_preferences_open != show_editor_preferences_window_)
  {
    mark_settings_dirty();
  }

  PumpBridgeBuildUpdates();
  if (!bridge_build_in_progress_)
  {
    RefreshBuildBridgeStatus(false);
  }

  if (show_build_workflow_window_)
  {
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(editor_settings_.panel_build_workflow_width), static_cast<float>(editor_settings_.panel_build_workflow_height)), ImGuiCond_FirstUseEver);
    ImGui::Begin("Build Workflow", &show_build_workflow_window_);
    ImGui::Text("Build and run TestECS");
    ImGui::Separator();

    std::vector<const char *> preset_labels;
    preset_labels.reserve(compiler_presets_.size());
    for (const auto &preset : compiler_presets_)
    {
      preset_labels.push_back(preset.name.c_str());
    }

    if (ImGui::Combo("Compiler preset", &selected_preset_index_, preset_labels.data(), static_cast<int>(preset_labels.size())))
    {
      editor_settings_.last_compiler_preset = compiler_presets_[selected_preset_index_].name;
      mark_settings_dirty();
    }

    char build_dir_buffer[1024];
    std::strncpy(build_dir_buffer, build_dir_input_.c_str(), sizeof(build_dir_buffer) - 1);
    build_dir_buffer[sizeof(build_dir_buffer) - 1] = '\0';
    if (ImGui::InputText("Build directory", build_dir_buffer, sizeof(build_dir_buffer)))
    {
      build_dir_input_ = build_dir_buffer;
      editor_settings_.last_build_dir = build_dir_input_;
      mark_settings_dirty();
    }

    char executable_buffer[1024];
    std::strncpy(executable_buffer, executable_input_.c_str(), sizeof(executable_buffer) - 1);
    executable_buffer[sizeof(executable_buffer) - 1] = '\0';
    if (ImGui::InputText("Executable path", executable_buffer, sizeof(executable_buffer)))
    {
      executable_input_ = executable_buffer;
      editor_settings_.last_executable_path = executable_input_;
      mark_settings_dirty();
    }
    if (ImGui::Button("Refresh paths"))
    {
      RefreshPaths(true);
      editor_settings_.last_build_dir = build_dir_input_;
      editor_settings_.last_executable_path = executable_input_;
      mark_settings_dirty();
    }

    ImGui::Separator();
    ImGui::BeginDisabled(bridge_build_in_progress_);
    if (ImGui::Button("Build TestECS"))
    {
      run_after_build_request_ = false;
      BuildTestECS();
    }
    ImGui::SameLine();
    if (ImGui::Button("Build && Run"))
    {
      run_after_build_request_ = true;
      BuildTestECS();
      if (!Editor::ShouldUseSocketBuilds(editor_settings_) && build_succeeded_)
      {
        run_after_build_request_ = false;
        RunTestECS();
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop"))
    {
      StopGame();
    }
    ImGui::EndDisabled();
    if (bridge_build_in_progress_)
    {
      ImGui::TextDisabled("A bridge build is already running.");
    }
    if (bridge_rebuild_needed_)
    {
      ImGui::TextColored(ImVec4(0.95f, 0.75f, 0.25f, 1.0f), "Rebuild needed: %d changed file(s) detected.", bridge_changed_file_count_);
    }

    ImGui::Separator();
    ImGui::TextWrapped("Status: %s", status_.c_str());
    ImGui::Text("Project root: %s", project_root_.string().c_str());
    ImGui::Text("Build dir: %s", build_dir_.empty() ? "<unset>" : build_dir_.string().c_str());
    ImGui::Text("Executable: %s", game_executable_.empty() ? "not found" : game_executable_.string().c_str());
    ImGui::Text("Game process ID: %lld", static_cast<long long>(game_pid_));
    if (build_bridge_warning_active_)
    {
      ImGui::TextColored(ImVec4(0.95f, 0.45f, 0.30f, 1.0f), "Bridge warning: connection probes are failing.");
    }

    const ImVec2 build_workflow_size = ImGui::GetWindowSize();
    const int build_workflow_width = std::max(1, static_cast<int>(build_workflow_size.x));
    const int build_workflow_height = std::max(1, static_cast<int>(build_workflow_size.y));
    if (editor_settings_.panel_build_workflow_width != build_workflow_width || editor_settings_.panel_build_workflow_height != build_workflow_height)
    {
      editor_settings_.panel_build_workflow_width = build_workflow_width;
      editor_settings_.panel_build_workflow_height = build_workflow_height;
      mark_settings_dirty();
    }

    ImGui::End();
  }

  if (show_preview_window_)
  {
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(editor_settings_.panel_preview_width), static_cast<float>(editor_settings_.panel_preview_height)), ImGuiCond_FirstUseEver);
    ImGui::Begin("Preview", &show_preview_window_);
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
    ImGui::BeginDisabled(bridge_build_in_progress_);
    if (ImGui::Button("Rebuild"))
    {
      run_after_build_request_ = false;
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

    if (preview_game_ && preview_game_->IsEngineRunning())
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

      if (!preview_texture_ready_ || preview_texture_.texture.width != tex_w || preview_texture_.texture.height != tex_h)
      {
        preview_texture_ = LoadRenderTexture(tex_w, tex_h);
        preview_texture_ready_ = true;
      }

      const double now = GetTime();
      const double preview_tick_interval = 1.0 / 60.0;
      if (preview_last_tick_time_ <= 0.0 || now - preview_last_tick_time_ >= preview_tick_interval)
      {
        preview_game_->TickHostedFrame();
        preview_last_tick_time_ = now;
      }

      BeginTextureMode(preview_texture_);
      ClearBackground(BLACK);
      preview_game_->RenderHostedFrame();
      EndTextureMode();

      const float offset_x = std::max(0.0f, (content_w - display_w) * 0.5f);
      ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset_x);
      ImGui::Image((ImTextureID)(intptr_t)preview_texture_.texture.id, ImVec2(display_w, display_h), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

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
    if (editor_settings_.panel_preview_width != preview_width || editor_settings_.panel_preview_height != preview_height)
    {
      editor_settings_.panel_preview_width = preview_width;
      editor_settings_.panel_preview_height = preview_height;
      mark_settings_dirty();
    }

    ImGui::End();
  }

  if (show_scene_window_)
  {
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(editor_settings_.panel_scene_width), static_cast<float>(editor_settings_.panel_scene_height)), ImGuiCond_FirstUseEver);
    ImGui::Begin("Scene", &show_scene_window_);
    ImGui::TextWrapped("Scene view placeholder. This panel is intended for object placement, transform gizmos, and component editing workflows.");
    ImGui::Separator();
    ImGui::BeginChild("SceneViewportPlaceholder", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_NoScrollbar);
    ImGui::TextDisabled("Scene viewport rendering target will appear here.");
    ImGui::EndChild();

    const ImVec2 scene_size = ImGui::GetWindowSize();
    const int scene_width = std::max(1, static_cast<int>(scene_size.x));
    const int scene_height = std::max(1, static_cast<int>(scene_size.y));
    if (editor_settings_.panel_scene_width != scene_width || editor_settings_.panel_scene_height != scene_height)
    {
      editor_settings_.panel_scene_width = scene_width;
      editor_settings_.panel_scene_height = scene_height;
      mark_settings_dirty();
    }

    ImGui::End();
  }

  if (show_console_window_)
  {
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(editor_settings_.panel_console_width), static_cast<float>(editor_settings_.panel_console_height)), ImGuiCond_FirstUseEver);
    ImGui::Begin("Console", &show_console_window_);
    if (ImGui::Button("Clear"))
    {
      console_lines_.clear();
    }
    ImGui::SameLine();
    ImGui::Text("%zu lines", console_lines_.size());

    ImGui::BeginChild("ConsoleScroll", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto &line : console_lines_)
    {
      RenderAnsiTextLine(line);
    }
    if (console_scroll_to_bottom_)
    {
      ImGui::SetScrollHereY(1.0f);
      console_scroll_to_bottom_ = false;
    }
    ImGui::EndChild();

    const ImVec2 console_size = ImGui::GetWindowSize();
    const int console_width = std::max(1, static_cast<int>(console_size.x));
    const int console_height = std::max(1, static_cast<int>(console_size.y));
    if (editor_settings_.panel_console_width != console_width || editor_settings_.panel_console_height != console_height)
    {
      editor_settings_.panel_console_width = console_width;
      editor_settings_.panel_console_height = console_height;
      mark_settings_dirty();
    }

    ImGui::End();
  }

  if (show_game_log_window_)
  {
    ImGui::SetNextWindowSize(ImVec2(720.0f, 240.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("Game Log", &show_game_log_window_);
    if (ImGui::Button("Clear"))
    {
      game_log_lines_.clear();
    }
    ImGui::SameLine();
    ImGui::Text("%zu lines", game_log_lines_.size());

    ImGui::BeginChild("GameLogScroll", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto &line : game_log_lines_)
    {
      RenderAnsiTextLine(line);
    }
    if (game_log_scroll_to_bottom_)
    {
      ImGui::SetScrollHereY(1.0f);
      game_log_scroll_to_bottom_ = false;
    }
    ImGui::EndChild();
    ImGui::End();
  }

  if (show_editor_preferences_window_)
  {
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(editor_settings_.panel_preferences_width), static_cast<float>(editor_settings_.panel_preferences_height)), ImGuiCond_FirstUseEver);
    ImGui::Begin("Editor Preferences", &show_editor_preferences_window_);

    ImGui::Text("Theme");
    const int safe_theme_index = std::max(0, std::min(selected_theme_preset_index_, static_cast<int>(theme_presets_.size()) - 1));
    const char *current_theme_label = theme_presets_.empty() ? "<none>" : theme_presets_[safe_theme_index].name.c_str();
    if (ImGui::BeginCombo("##ThemePreset", current_theme_label))
    {
      for (int i = 0; i < static_cast<int>(theme_presets_.size()); ++i)
      {
        const bool selected = i == selected_theme_preset_index_;
        if (ImGui::Selectable(theme_presets_[i].name.c_str(), selected))
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

    int font_scale_percent = static_cast<int>(ui_font_scale_ * 100.0f + 0.5f);
    if (ImGui::SliderInt("Font scale (%)", &font_scale_percent, 80, 180))
    {
      ui_font_scale_ = static_cast<float>(font_scale_percent) / 100.0f;
      ApplyThemeCustomizations();
      PersistThemePreference();
    }

    int spacing_percent = static_cast<int>(ui_spacing_density_ * 100.0f + 0.5f);
    if (ImGui::SliderInt("Spacing density (%)", &spacing_percent, 70, 150))
    {
      ui_spacing_density_ = static_cast<float>(spacing_percent) / 100.0f;
      ApplyThemeCustomizations();
      PersistThemePreference();
    }

    int rounding_value = static_cast<int>(ui_rounding_ + 0.5f);
    if (ImGui::SliderInt("Corner rounding", &rounding_value, 0, 16))
    {
      ui_rounding_ = static_cast<float>(rounding_value);
      ApplyThemeCustomizations();
      PersistThemePreference();
    }

    float accent_rgb[3] = {ui_accent_color_.x, ui_accent_color_.y, ui_accent_color_.z};
    if (ImGui::ColorEdit3("Accent color", accent_rgb))
    {
      ui_accent_color_.x = accent_rgb[0];
      ui_accent_color_.y = accent_rgb[1];
      ui_accent_color_.z = accent_rgb[2];
      ui_accent_color_.w = 1.0f;
      ApplyThemeCustomizations();
      PersistThemePreference();
    }

    ImGui::Separator();
    ImGui::Text("Layout");
#ifdef IMGUI_HAS_DOCK
    if (layout_presets_.empty())
    {
      layout_presets_ = DefaultLayoutPresets();
    }

    const int safe_layout_index = std::max(0, std::min(selected_layout_preset_index_, static_cast<int>(layout_presets_.size()) - 1));
    const char *current_layout_label = layout_presets_.empty() ? "<none>" : layout_presets_[safe_layout_index].name.c_str();
    if (ImGui::BeginCombo("##DockLayoutPreset", current_layout_label))
    {
      for (int i = 0; i < static_cast<int>(layout_presets_.size()); ++i)
      {
        const bool selected = i == selected_layout_preset_index_;
        if (ImGui::Selectable(layout_presets_[i].name.c_str(), selected))
        {
          selected_layout_preset_index_ = i;
          dock_layout_apply_requested_ = true;
          editor_settings_.layout_preset_index = selected_layout_preset_index_;
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
      dock_layout_apply_requested_ = true;
      editor_settings_.layout_preset_index = selected_layout_preset_index_;
      mark_settings_dirty();
    }
#else
    ImGui::TextDisabled("Docking not enabled in this build.");
#endif

    const ImVec2 preferences_size = ImGui::GetWindowSize();
    const int preferences_width = std::max(1, static_cast<int>(preferences_size.x));
    const int preferences_height = std::max(1, static_cast<int>(preferences_size.y));
    if (editor_settings_.panel_preferences_width != preferences_width || editor_settings_.panel_preferences_height != preferences_height)
    {
      editor_settings_.panel_preferences_width = preferences_width;
      editor_settings_.panel_preferences_height = preferences_height;
      mark_settings_dirty();
    }

    ImGui::End();
  }

  if (settings_dirty)
  {
    editor_settings_.panel_build_workflow_open = show_build_workflow_window_;
    editor_settings_.panel_scene_open = show_scene_window_;
    editor_settings_.panel_preview_open = show_preview_window_;
    editor_settings_.panel_console_open = show_console_window_;
    editor_settings_.panel_game_log_open = show_game_log_window_;
    editor_settings_.panel_preferences_open = show_editor_preferences_window_;
    editor_settings_.last_compiler_preset = compiler_presets_[std::max(0, std::min(selected_preset_index_, static_cast<int>(compiler_presets_.size()) - 1))].name;
    editor_settings_.last_build_dir = build_dir_input_;
    editor_settings_.last_executable_path = executable_input_;
    editor_settings_.layout_preset_index = selected_layout_preset_index_;
    if (!Editor::SaveEditorSettings(project_root_, editor_settings_))
    {
      AppendConsoleLine("Warning: Failed to persist editor workspace settings.");
    }
  }

  rlImGuiEnd();
}

void MyriadEditor::AppendConsoleLine(const std::string &line)
{
  console_lines_.push_back(line);
  console_scroll_to_bottom_ = true;
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
  if (bridge_build_thread_.joinable())
  {
    bridge_build_thread_.join();
  }
}

void MyriadEditor::RestartPreviewForLatestBuild()
{
  hosted_library_reload_required_ = true;
  if (!preview_game_ || !preview_game_->IsEngineRunning())
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

  hosted_library_path_ = FindHostedLibraryPath(build_dir_);
  if (hosted_library_path_.empty())
  {
    AppendConsoleLine("Hosted preview library was not found in build output.");
    return false;
  }

  std::error_code fs_error;
  const std::filesystem::path cache_dir = build_dir_ / "Editor" / ".myriad_preview_cache";
  std::filesystem::create_directories(cache_dir, fs_error);
  if (fs_error)
  {
    AppendConsoleLine("Failed to prepare preview cache directory: " + fs_error.message());
    return false;
  }

  const auto now_ticks = std::chrono::duration_cast<std::chrono::microseconds>(
                             std::chrono::system_clock::now().time_since_epoch())
                             .count();
  hosted_library_loaded_copy_path_ = cache_dir / (hosted_library_path_.stem().string() + "_preview_" + std::to_string(now_ticks) + hosted_library_path_.extension().string());

  std::filesystem::copy_file(hosted_library_path_, hosted_library_loaded_copy_path_, std::filesystem::copy_options::overwrite_existing, fs_error);
  if (fs_error)
  {
    AppendConsoleLine("Failed to stage hosted preview library copy: " + fs_error.message());
    hosted_library_loaded_copy_path_.clear();
    return false;
  }

#ifdef _WIN32
  HMODULE module = LoadLibraryA(hosted_library_loaded_copy_path_.string().c_str());
  if (module == nullptr)
  {
    AppendConsoleLine("Failed to load hosted preview library: " + hosted_library_loaded_copy_path_.string());
    hosted_library_loaded_copy_path_.clear();
    return false;
  }

  using HostedCreateFn = Myriad::MyrGameApplication *(*)();
  HostedCreateFn create_fn = reinterpret_cast<HostedCreateFn>(GetProcAddress(module, "Myriad_CreateHostedGame"));
  hosted_destroy_fn_ = reinterpret_cast<HostedDestroyFn>(GetProcAddress(module, "Myriad_DestroyHostedGame"));
  if (create_fn == nullptr || hosted_destroy_fn_ == nullptr)
  {
    AppendConsoleLine("Hosted preview symbols were not found in library.");
    FreeLibrary(module);
    hosted_library_loaded_copy_path_.clear();
    return false;
  }

  preview_game_ = create_fn();
  if (preview_game_ == nullptr)
  {
    AppendConsoleLine("Hosted preview factory returned no game instance.");
    FreeLibrary(module);
    hosted_destroy_fn_ = nullptr;
    hosted_library_loaded_copy_path_.clear();
    return false;
  }

  hosted_library_handle_ = module;
#else
  void *module = dlopen(hosted_library_loaded_copy_path_.c_str(), RTLD_NOW | RTLD_LOCAL);
  if (module == nullptr)
  {
    AppendConsoleLine(std::string("Failed to load hosted preview library: ") + dlerror());
    hosted_library_loaded_copy_path_.clear();
    return false;
  }

  using HostedCreateFn = Myriad::MyrGameApplication *(*)();
  HostedCreateFn create_fn = reinterpret_cast<HostedCreateFn>(dlsym(module, "Myriad_CreateHostedGame"));
  hosted_destroy_fn_ = reinterpret_cast<HostedDestroyFn>(dlsym(module, "Myriad_DestroyHostedGame"));
  if (create_fn == nullptr || hosted_destroy_fn_ == nullptr)
  {
    AppendConsoleLine("Hosted preview symbols were not found in library.");
    dlclose(module);
    hosted_library_loaded_copy_path_.clear();
    return false;
  }

  preview_game_ = create_fn();
  if (preview_game_ == nullptr)
  {
    AppendConsoleLine("Hosted preview factory returned no game instance.");
    dlclose(module);
    hosted_destroy_fn_ = nullptr;
    hosted_library_loaded_copy_path_.clear();
    return false;
  }

  hosted_library_handle_ = module;
#endif

  hosted_library_reload_required_ = false;
  AppendConsoleLine("Hosted preview library loaded: " + hosted_library_path_.string());
  return true;
}

void MyriadEditor::UnloadHostedPreviewLibrary()
{
  if (preview_game_ != nullptr)
  {
    if (preview_game_->IsEngineRunning())
    {
      preview_game_->StopHosted();
    }

    if (hosted_destroy_fn_ != nullptr)
    {
      hosted_destroy_fn_(preview_game_);
    }
    else
    {
      delete preview_game_;
    }
    preview_game_ = nullptr;
  }

  hosted_destroy_fn_ = nullptr;
  if (hosted_library_handle_ != nullptr)
  {
#ifdef _WIN32
    FreeLibrary(static_cast<HMODULE>(hosted_library_handle_));
#else
    dlclose(hosted_library_handle_);
#endif
    hosted_library_handle_ = nullptr;
  }

  if (!hosted_library_loaded_copy_path_.empty())
  {
    std::error_code fs_error;
    std::filesystem::remove(hosted_library_loaded_copy_path_, fs_error);
    hosted_library_loaded_copy_path_.clear();
  }
}

void MyriadEditor::StartBridgeBuildOverSocket(const Editor::CompilerPreset &preset, const std::string &build_dir_relative)
{
  if (bridge_build_in_progress_)
  {
    status_ = "A build bridge request is already in progress.";
    AppendConsoleLine(status_);
    return;
  }

  JoinBridgeBuildThread();

  {
    std::lock_guard<std::mutex> lock(bridge_build_mutex_);
    bridge_build_pending_lines_.clear();
    bridge_build_result_ready_ = false;
    bridge_build_request_success_ = false;
    bridge_build_success_ = false;
    bridge_build_rebuild_triggered_ = false;
    bridge_build_response_.clear();
    bridge_build_error_.clear();
    bridge_build_status_.clear();
    bridge_build_executable_.clear();
    bridge_build_host_.clear();
  }

  bridge_build_in_progress_ = true;
  status_ = "Building TestECS through the socket bridge...";
  build_succeeded_ = false;
  bridge_build_progress_percent_ = -1;

  const std::string request = Editor::CreateBuildBridgeRequest(preset, build_dir_relative);
  const std::string requested_host = editor_settings_.build_socket_host.empty() ? std::string{"auto"} : editor_settings_.build_socket_host;
  AppendConsoleLine("Sending bridge request to " + requested_host + ":" + std::to_string(editor_settings_.build_socket_port));

  Editor::EditorSettings settings_copy = editor_settings_;

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
          bridge_build_pending_lines_.push_back(line);
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
      bridge_build_request_success_ = request_success;
      bridge_build_success_ = bridge_success;
      bridge_build_rebuild_triggered_ = rebuild_triggered;
      bridge_build_response_ = response;
      bridge_build_error_ = bridge_error;
      bridge_build_status_ = bridge_status;
      bridge_build_executable_ = bridge_executable;
      bridge_build_host_ = settings_copy.build_socket_host;
      bridge_build_result_ready_ = true;
    }

    bridge_build_in_progress_ = false; });
}

void MyriadEditor::UpdateBridgeBuildProgressFromLine(const std::string &line)
{
  int parsed_percent = -1;
  if (!TryExtractBuildProgressPercent(line, parsed_percent))
  {
    return;
  }

  if (parsed_percent > bridge_build_progress_percent_)
  {
    bridge_build_progress_percent_ = parsed_percent;
  }
}

void MyriadEditor::PumpBridgeBuildUpdates()
{
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
    pending_lines.swap(bridge_build_pending_lines_);
    if (bridge_build_result_ready_)
    {
      has_result = true;
      request_success = bridge_build_request_success_;
      bridge_success = bridge_build_success_;
      rebuild_triggered = bridge_build_rebuild_triggered_;
      response = bridge_build_response_;
      bridge_error = bridge_build_error_;
      bridge_status = bridge_build_status_;
      bridge_executable = bridge_build_executable_;
      resolved_host = bridge_build_host_;
      bridge_build_result_ready_ = false;
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
    editor_settings_.build_socket_host = resolved_host;
  }

  if (!request_success)
  {
    build_succeeded_ = false;
    run_after_build_request_ = false;
    build_bridge_connected_ = false;
    build_bridge_status_text_ = bridge_error;
    status_ = "Build bridge error: " + bridge_error;
    AppendConsoleLine(status_);
    bridge_build_progress_percent_ = -1;
    return;
  }

  bridge_rebuild_needed_ = Editor::ExtractJsonBool(response, "rebuildNeeded", bridge_rebuild_needed_);
  bridge_changed_file_count_ = std::max(0, Editor::ExtractJsonInt(response, "changedFileCount", bridge_changed_file_count_));
  bridge_changed_files_preview_ = ExtractJsonStringArray(response, "changedFilesPreview", 12);

  build_bridge_connected_ = bridge_success;
  build_bridge_status_text_ = bridge_status.empty() ? "Bridge reachable." : bridge_status;

  if (!bridge_success)
  {
    build_succeeded_ = false;
    run_after_build_request_ = false;
    status_ = bridge_status.empty() ? "The build container reported a failure." : bridge_status;
    AppendConsoleLine(status_);
    bridge_build_progress_percent_ = -1;
    return;
  }

  build_succeeded_ = true;
  bridge_build_progress_percent_ = 100;
  bridge_rebuild_needed_ = false;
  bridge_changed_file_count_ = 0;
  if (!bridge_executable.empty())
  {
    game_executable_ = Editor::ResolvePath(project_root_, std::filesystem::path(bridge_executable));
  }
  else
  {
    game_executable_ = Editor::FindGameExecutable(build_dir_, executable_input_, project_root_);
  }

  executable_input_ = game_executable_.empty() ? executable_input_ : game_executable_.string();
  status_ = bridge_status.empty() ? "Build succeeded through the socket bridge." : bridge_status;
  AppendConsoleLine(status_);

  (void)rebuild_triggered;
  RestartPreviewForLatestBuild();

  if (run_after_build_request_)
  {
    run_after_build_request_ = false;
    RunTestECS();
  }
}

void MyriadEditor::ApplyThemePresetByIndex(int preset_index)
{
  if (theme_presets_.empty())
  {
    theme_presets_ = DefaultThemePresets();
  }

  if (preset_index < 0 || preset_index >= static_cast<int>(theme_presets_.size()))
  {
    preset_index = 0;
  }

  selected_theme_preset_index_ = preset_index;
  const Editor::ThemePreset &preset = theme_presets_[preset_index];
  editor_settings_.theme_preset = preset.name;

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

  base_theme_style_ = ImGui::GetStyle();
  has_base_theme_style_ = true;
}

void MyriadEditor::ApplyThemeCustomizations()
{
  if (!has_base_theme_style_)
  {
    return;
  }

  ImGuiStyle tuned = base_theme_style_;

  auto scale_vec2 = [this](const ImVec2 &value)
  {
    return ImVec2(value.x * ui_spacing_density_, value.y * ui_spacing_density_);
  };

  tuned.WindowPadding = scale_vec2(tuned.WindowPadding);
  tuned.FramePadding = scale_vec2(tuned.FramePadding);
  tuned.ItemSpacing = scale_vec2(tuned.ItemSpacing);
  tuned.ItemInnerSpacing = scale_vec2(tuned.ItemInnerSpacing);
  tuned.CellPadding = scale_vec2(tuned.CellPadding);
  tuned.TouchExtraPadding = scale_vec2(tuned.TouchExtraPadding);

  tuned.WindowRounding = ui_rounding_;
  tuned.ChildRounding = ui_rounding_;
  tuned.PopupRounding = ui_rounding_;
  tuned.FrameRounding = std::max(0.0f, ui_rounding_ * 0.65f);
  tuned.GrabRounding = std::max(0.0f, ui_rounding_ * 0.5f);
  tuned.ScrollbarRounding = std::max(0.0f, ui_rounding_ * 0.5f);
  tuned.TabRounding = std::max(0.0f, ui_rounding_ * 0.5f);

  const ImVec4 accent_brighter = ShiftBrightness(ui_accent_color_, 0.12f);
  const ImVec4 accent_hover = ShiftBrightness(ui_accent_color_, 0.10f);
  const ImVec4 accent_darker = ShiftBrightness(ui_accent_color_, -0.08f);
  const ImVec4 accent_active = ShiftBrightness(ui_accent_color_, -0.05f);

  tuned.Colors[ImGuiCol_Button] = ImVec4(ui_accent_color_.x, ui_accent_color_.y, ui_accent_color_.z, 0.78f);
  tuned.Colors[ImGuiCol_ButtonHovered] = ImVec4(accent_brighter.x, accent_brighter.y, accent_brighter.z, 0.88f);
  tuned.Colors[ImGuiCol_ButtonActive] = ImVec4(accent_darker.x, accent_darker.y, accent_darker.z, 0.92f);
  tuned.Colors[ImGuiCol_Header] = ImVec4(ui_accent_color_.x, ui_accent_color_.y, ui_accent_color_.z, 0.58f);
  tuned.Colors[ImGuiCol_HeaderHovered] = ImVec4(accent_hover.x, accent_hover.y, accent_hover.z, 0.72f);
  tuned.Colors[ImGuiCol_HeaderActive] = ImVec4(accent_active.x, accent_active.y, accent_active.z, 0.86f);
  tuned.Colors[ImGuiCol_CheckMark] = ui_accent_color_;
  tuned.Colors[ImGuiCol_SliderGrab] = ui_accent_color_;
  tuned.Colors[ImGuiCol_SliderGrabActive] = accent_brighter;
  tuned.Colors[ImGuiCol_TabActive] = ImVec4(ui_accent_color_.x, ui_accent_color_.y, ui_accent_color_.z, 0.82f);
  tuned.Colors[ImGuiCol_TabHovered] = ImVec4(accent_brighter.x, accent_brighter.y, accent_brighter.z, 0.90f);

  ImGui::GetStyle() = tuned;
  ImGui::GetIO().FontGlobalScale = ui_font_scale_;
}

void MyriadEditor::PersistThemePreference()
{
  editor_settings_.ui_font_scale_percent = static_cast<int>(ui_font_scale_ * 100.0f + 0.5f);
  editor_settings_.ui_rounding = static_cast<int>(ui_rounding_ + 0.5f);
  editor_settings_.ui_spacing_percent = static_cast<int>(ui_spacing_density_ * 100.0f + 0.5f);
  editor_settings_.ui_accent_hex = ToHexColor(ui_accent_color_);

  if (!Editor::SaveEditorSettings(project_root_, editor_settings_))
  {
    AppendConsoleLine("Warning: Failed to save theme preference to .myriad-editor.json");
  }
}

void MyriadEditor::StartPreviewGame()
{
  if (preview_game_ && preview_game_->IsEngineRunning())
  {
    return;
  }

  StopPreviewGame();

  // Always reload on Play so hosted preview picks up any externally rebuilt library.
  if (!ReloadHostedPreviewLibrary())
  {
    status_ = "Failed to reload hosted preview library.";
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
  strncpy(config.window_title, "Embedded Preview", sizeof(config.window_title) - 1);

  if (preview_game_ != nullptr && preview_game_->StartHosted(config, false))
  {
    preview_texture_ = LoadRenderTexture(800, 600);
    preview_texture_ready_ = true;
    status_ = "Embedded preview running.";
  }
  else
  {
    EndPreviewLogCapture();
    UnloadHostedPreviewLibrary();
    hosted_library_reload_required_ = true;
    status_ = "Failed to start embedded preview.";
  }
}

void MyriadEditor::StopPreviewGame()
{
  if (preview_game_)
  {
    if (preview_game_->IsEngineRunning())
    {
      preview_game_->StopHosted();
    }
  }

  preview_texture_ = {};
  preview_texture_ready_ = false;
  preview_stop_requested_ = false;
  preview_texture_cleanup_requested_ = false;
  preview_last_tick_time_ = 0.0;
  EndPreviewLogCapture();
}

void MyriadEditor::PumpGameLogOutput()
{
#ifdef _WIN32
  // TODO: Add Windows preview log stream capture using named pipes.
#else
  if (preview_log_pipe_read_fd_ < 0)
  {
    return;
  }

  char buffer[1024];
  while (true)
  {
    const ssize_t bytes_read = read(preview_log_pipe_read_fd_, buffer, sizeof(buffer));
    if (bytes_read <= 0)
    {
      break;
    }

    preview_log_partial_line_.append(buffer, static_cast<std::size_t>(bytes_read));
    std::size_t line_end = std::string::npos;
    while ((line_end = preview_log_partial_line_.find('\n')) != std::string::npos)
    {
      std::string line = preview_log_partial_line_.substr(0, line_end);
      if (!line.empty() && line.back() == '\r')
      {
        line.pop_back();
      }
      game_log_lines_.push_back(line);
      game_log_scroll_to_bottom_ = true;
      preview_log_partial_line_.erase(0, line_end + 1);
    }
  }
#endif
}

void MyriadEditor::BeginPreviewLogCapture()
{
#ifdef _WIN32
  return;
#else
  if (preview_log_pipe_read_fd_ >= 0)
  {
    return;
  }

  int pipe_fds[2] = {-1, -1};
  if (pipe(pipe_fds) != 0)
  {
    game_log_lines_.push_back("[editor] failed to initialize preview log capture pipe");
    game_log_scroll_to_bottom_ = true;
    return;
  }

  preview_log_saved_stdout_fd_ = dup(STDOUT_FILENO);
  preview_log_saved_stderr_fd_ = dup(STDERR_FILENO);
  if (preview_log_saved_stdout_fd_ < 0 || preview_log_saved_stderr_fd_ < 0)
  {
    close(pipe_fds[0]);
    close(pipe_fds[1]);
    preview_log_saved_stdout_fd_ = -1;
    preview_log_saved_stderr_fd_ = -1;
    game_log_lines_.push_back("[editor] failed to duplicate stdout/stderr for preview capture");
    game_log_scroll_to_bottom_ = true;
    return;
  }

  if (dup2(pipe_fds[1], STDOUT_FILENO) < 0 || dup2(pipe_fds[1], STDERR_FILENO) < 0)
  {
    close(pipe_fds[0]);
    close(pipe_fds[1]);
    close(preview_log_saved_stdout_fd_);
    close(preview_log_saved_stderr_fd_);
    preview_log_saved_stdout_fd_ = -1;
    preview_log_saved_stderr_fd_ = -1;
    game_log_lines_.push_back("[editor] failed to redirect stdout/stderr for preview capture");
    game_log_scroll_to_bottom_ = true;
    return;
  }

  const int flags = fcntl(pipe_fds[0], F_GETFL, 0);
  if (flags >= 0)
  {
    fcntl(pipe_fds[0], F_SETFL, flags | O_NONBLOCK);
  }

  preview_log_pipe_read_fd_ = pipe_fds[0];
  preview_log_pipe_write_fd_ = pipe_fds[1];
  preview_log_partial_line_.clear();
  game_log_lines_.push_back("[editor] preview log capture started");
  game_log_scroll_to_bottom_ = true;
#endif
}

void MyriadEditor::EndPreviewLogCapture()
{
#ifdef _WIN32
  return;
#else
  if (preview_log_pipe_read_fd_ < 0)
  {
    return;
  }

  fflush(stdout);
  fflush(stderr);

  if (preview_log_saved_stdout_fd_ >= 0)
  {
    dup2(preview_log_saved_stdout_fd_, STDOUT_FILENO);
    close(preview_log_saved_stdout_fd_);
    preview_log_saved_stdout_fd_ = -1;
  }
  if (preview_log_saved_stderr_fd_ >= 0)
  {
    dup2(preview_log_saved_stderr_fd_, STDERR_FILENO);
    close(preview_log_saved_stderr_fd_);
    preview_log_saved_stderr_fd_ = -1;
  }

  PumpGameLogOutput();
  if (!preview_log_partial_line_.empty())
  {
    game_log_lines_.push_back(preview_log_partial_line_);
    game_log_scroll_to_bottom_ = true;
    preview_log_partial_line_.clear();
  }

  if (preview_log_pipe_write_fd_ >= 0)
  {
    close(preview_log_pipe_write_fd_);
    preview_log_pipe_write_fd_ = -1;
  }
  if (preview_log_pipe_read_fd_ >= 0)
  {
    close(preview_log_pipe_read_fd_);
    preview_log_pipe_read_fd_ = -1;
  }

  game_log_lines_.push_back("[editor] preview log capture stopped");
  game_log_scroll_to_bottom_ = true;
#endif
}

void MyriadEditor::RefreshPaths(bool force_defaults)
{
  if (compiler_presets_.empty())
  {
    compiler_presets_.push_back({"Default", {}});
  }

  const auto &preset = compiler_presets_[std::max(0, std::min(selected_preset_index_, static_cast<int>(compiler_presets_.size()) - 1))];
  const std::filesystem::path detected_build_dir = Editor::FindMatchingBuildDirectory(project_root_, preset.name);
  const std::filesystem::path default_build_dir = detected_build_dir.empty() ? Editor::ResolvePath(project_root_, std::filesystem::path("build") / preset.name / "Debug") : detected_build_dir;

  std::filesystem::path selected_build_dir = force_defaults || build_dir_input_.empty() ? default_build_dir : std::filesystem::path(build_dir_input_);
  if (!selected_build_dir.is_absolute())
  {
    selected_build_dir = Editor::ResolvePath(project_root_, selected_build_dir);
  }

  build_dir_ = std::filesystem::absolute(selected_build_dir);
  build_dir_input_ = build_dir_.string();

  if (force_defaults || executable_input_.empty())
  {
    game_executable_ = Editor::FindGameExecutable(build_dir_, {}, project_root_);
  }
  else
  {
    const std::filesystem::path resolved_override = Editor::ResolveExecutablePath(build_dir_, project_root_, std::filesystem::path(executable_input_));
    game_executable_ = Editor::FindGameExecutable(build_dir_, resolved_override.empty() ? std::filesystem::path{} : resolved_override, project_root_);
  }

  executable_input_ = game_executable_.empty() ? "" : game_executable_.string();
  status_ = "Paths refreshed for the selected compiler preset.";
}

void MyriadEditor::RefreshBuildBridgeStatus(bool force)
{
  if (!Editor::ShouldUseSocketBuilds(editor_settings_))
  {
    Editor::DisconnectBuildBridge();
    build_bridge_connected_ = false;
    build_bridge_status_text_ = "Local build mode.";
    build_bridge_consecutive_failures_ = 0;
    build_bridge_warning_active_ = false;
    build_bridge_last_probe_time_ = GetTime();
    return;
  }

  const double now = GetTime();
  const double probe_interval_seconds = static_cast<double>(std::max(5, std::min(3600, editor_settings_.build_bridge_probe_interval_seconds)));
  if (!force)
  {
    if (now - build_bridge_last_probe_time_ < probe_interval_seconds)
    {
      return;
    }
  }

  build_bridge_last_probe_time_ = now;
  std::string response;
  std::string bridge_error;
  if (Editor::ProbeBuildBridge(editor_settings_, response, bridge_error))
  {
    const bool bridge_success = Editor::ExtractJsonBool(response, "success", true);
    const std::string bridge_status = Editor::ExtractJsonString(response, "status");
    const bool rebuild_needed = Editor::ExtractJsonBool(response, "rebuildNeeded", false);
    const int changed_file_count = Editor::ExtractJsonInt(response, "changedFileCount", 0);
    const std::vector<std::string> changed_files_preview = ExtractJsonStringArray(response, "changedFilesPreview", 12);

    const bool had_rebuild_needed = bridge_rebuild_needed_;
    bridge_rebuild_needed_ = rebuild_needed;
    bridge_changed_file_count_ = std::max(0, changed_file_count);
    bridge_changed_files_preview_ = changed_files_preview;

    build_bridge_connected_ = bridge_success;
    build_bridge_status_text_ = bridge_status.empty() ? "Bridge reachable." : bridge_status;
    if (bridge_success)
    {
      if (build_bridge_warning_active_)
      {
        AppendConsoleLine("Build bridge connectivity restored.");
      }
      build_bridge_consecutive_failures_ = 0;
      build_bridge_warning_active_ = false;
    }
    else
    {
      ++build_bridge_consecutive_failures_;
    }

    if (!had_rebuild_needed && bridge_rebuild_needed_)
    {
      AppendConsoleLine("Build bridge detected source changes. Rebuild is needed.");
    }
    else if (had_rebuild_needed && !bridge_rebuild_needed_)
    {
      AppendConsoleLine("Build bridge reports outputs are up to date.");
    }

    if (force)
    {
      AppendConsoleLine("Bridge status: " + build_bridge_status_text_);
    }
  }
  else
  {
    build_bridge_connected_ = false;
    bridge_rebuild_needed_ = false;
    bridge_changed_file_count_ = 0;
    bridge_changed_files_preview_.clear();
    build_bridge_status_text_ = bridge_error;
    ++build_bridge_consecutive_failures_;
    if (force)
    {
      AppendConsoleLine("Bridge status error: " + bridge_error);
    }
  }

  constexpr int warning_threshold = 3;
  if (!build_bridge_connected_ && build_bridge_consecutive_failures_ >= warning_threshold)
  {
    if (!build_bridge_warning_active_)
    {
      AppendConsoleLine("Warning: build bridge probe failed repeatedly; startup/builds may be degraded until connectivity is restored.");
    }
    build_bridge_warning_active_ = true;
  }
}

void MyriadEditor::BuildTestECS()
{
  build_succeeded_ = false;
  project_root_ = Editor::FindProjectRoot();
  const auto &preset = compiler_presets_[std::max(0, std::min(selected_preset_index_, static_cast<int>(compiler_presets_.size()) - 1))];
  const std::filesystem::path detected_build_dir = Editor::FindMatchingBuildDirectory(project_root_, preset.name);
  const std::filesystem::path default_build_dir = detected_build_dir.empty() ? (project_root_ / "build" / preset.name / "Debug") : detected_build_dir;
  build_dir_ = std::filesystem::absolute(build_dir_input_.empty() ? default_build_dir : std::filesystem::path(build_dir_input_));
  std::filesystem::create_directories(build_dir_);

  if (Editor::ShouldUseSocketBuilds(editor_settings_))
  {
    const std::string build_dir_relative = Editor::BuildDirForBridge(build_dir_, project_root_);
    if (build_dir_relative.empty())
    {
      build_succeeded_ = false;
      status_ = "Socket builds require the build directory to be inside the project root.";
      AppendConsoleLine(status_);
      return;
    }

    StartBridgeBuildOverSocket(preset, build_dir_relative);
    return;
  }

  std::filesystem::path toolchain_path;
  if (!preset.toolchain_file.empty())
  {
    const std::filesystem::path resolved_toolchain_path = Editor::ResolvePath(project_root_, preset.toolchain_file);
    if (std::filesystem::exists(resolved_toolchain_path))
    {
      toolchain_path = resolved_toolchain_path;
    }
  }

  std::string command = Editor::ExpandBuildCommandTemplate(build_command_template_, project_root_, build_dir_, preset, toolchain_path);
  std::cout << "Building TestECS with: " << command << std::endl;
  status_ = "Building TestECS...";
  AppendConsoleLine("Build request: " + preset.name + " -> " + build_dir_.string());

  const int result = std::system(command.c_str());
  if (result == 0)
  {
    build_succeeded_ = true;
    bridge_rebuild_needed_ = false;
    bridge_changed_file_count_ = 0;
    const std::filesystem::path detected_build_dir = Editor::FindMatchingBuildDirectory(project_root_, preset.name);
    const std::filesystem::path default_build_dir = detected_build_dir.empty() ? (project_root_ / "build" / preset.name / "Debug") : detected_build_dir;
    build_dir_ = std::filesystem::absolute(build_dir_input_.empty() ? default_build_dir : std::filesystem::path(build_dir_input_));
    game_executable_ = Editor::FindGameExecutable(build_dir_, executable_input_, project_root_);
    executable_input_ = game_executable_.empty() ? executable_input_ : game_executable_.string();
    status_ = "Build succeeded.";
    AppendConsoleLine("Build succeeded.");
    RestartPreviewForLatestBuild();
  }
  else
  {
    build_succeeded_ = false;
    status_ = "Build failed. Check the terminal output for details.";
    AppendConsoleLine("Build failed. Check the terminal output for details.");
  }
}

void MyriadEditor::RunTestECS()
{
  if (game_pid_ > 0)
  {
    status_ = "The TestECS process is already running.";
    return;
  }

  const std::filesystem::path existing_executable = game_executable_;
  if (existing_executable.empty() || !std::filesystem::exists(existing_executable))
  {
    const auto &preset = compiler_presets_[std::max(0, std::min(selected_preset_index_, static_cast<int>(compiler_presets_.size()) - 1))];
    const std::filesystem::path detected_build_dir = Editor::FindMatchingBuildDirectory(project_root_, preset.name);
    const std::filesystem::path default_build_dir = detected_build_dir.empty() ? (project_root_ / "build" / preset.name / "Debug") : detected_build_dir;
    build_dir_ = std::filesystem::absolute(build_dir_input_.empty() ? default_build_dir : std::filesystem::path(build_dir_input_));
    game_executable_ = Editor::FindGameExecutable(build_dir_, executable_input_, project_root_);
  }

  if (game_executable_.empty())
  {
    status_ = "Unable to locate the TestECS executable. Build it first.";
    return;
  }

  std::intptr_t process_id = -1;
  const std::filesystem::path runtime_log_dir = project_root_ / "runtime_logs";
  std::error_code runtime_log_error;
  std::filesystem::create_directories(runtime_log_dir, runtime_log_error);
  const std::filesystem::path runtime_log_path = runtime_log_dir / ("game-runtime-" + TimestampForLogPath() + ".log");

  if (Editor::LaunchProcess(game_executable_, project_root_, process_id, &runtime_log_path))
  {
    game_pid_ = process_id;
    status_ = "TestECS launched.";
    AppendConsoleLine("Launched game runtime log: " + runtime_log_path.string());
  }
  else
  {
    status_ = "Failed to launch TestECS.";
  }
}

void MyriadEditor::StopGame()
{
  if (game_pid_ > 0)
  {
    Editor::StopProcess(game_pid_);
    game_pid_ = -1;
    status_ = "Stopped the game process.";
  }
}
