#include "json_utils.h"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <limits>

namespace Editor
{
  std::string Trim(const std::string &value)
  {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
    {
      return {};
    }

    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
  }

  std::string ExtractJsonString(const std::string &text, const std::string &key)
  {
    const std::string pattern = "\"" + key + "\"";
    std::size_t key_pos = text.find(pattern);
    if (key_pos == std::string::npos)
    {
      return {};
    }

    const std::size_t value_pos = text.find(':', key_pos);
    if (value_pos == std::string::npos)
    {
      return {};
    }

    std::size_t first_quote = text.find('"', value_pos + 1);
    if (first_quote == std::string::npos)
    {
      return {};
    }

    std::size_t second_quote = text.find('"', first_quote + 1);
    if (second_quote == std::string::npos)
    {
      return {};
    }

    return text.substr(first_quote + 1, second_quote - first_quote - 1);
  }

  int ExtractJsonInt(const std::string &text, const std::string &key, int default_value)
  {
    const std::string pattern = "\"" + key + "\"";
    std::size_t key_pos = text.find(pattern);
    if (key_pos == std::string::npos)
    {
      return default_value;
    }

    const std::size_t value_pos = text.find(':', key_pos);
    if (value_pos == std::string::npos)
    {
      return default_value;
    }

    std::string token = Trim(text.substr(value_pos + 1));
    if (token.empty())
    {
      return default_value;
    }

    if (token.front() == '"')
    {
      const std::size_t second_quote = token.find('"', 1);
      if (second_quote == std::string::npos)
      {
        return default_value;
      }

      token = token.substr(1, second_quote - 1);
    }
    else
    {
      const std::size_t end = token.find_first_not_of("0123456789+-");
      if (end != std::string::npos)
      {
        token = token.substr(0, end);
      }
    }

    const char *begin = token.c_str();
    char *end = nullptr;
    errno = 0;
    const long parsed = std::strtol(begin, &end, 10);
    if (begin == end || errno == ERANGE || parsed < std::numeric_limits<int>::min() || parsed > std::numeric_limits<int>::max())
    {
      return default_value;
    }

    while (*end != '\0' && std::isspace(static_cast<unsigned char>(*end)) != 0)
    {
      ++end;
    }

    if (*end != '\0')
    {
      return default_value;
    }

    return static_cast<int>(parsed);
  }

  bool ExtractJsonBool(const std::string &text, const std::string &key, bool default_value)
  {
    const std::string pattern = "\"" + key + "\"";
    std::size_t key_pos = text.find(pattern);
    if (key_pos == std::string::npos)
    {
      return default_value;
    }

    const std::size_t value_pos = text.find(':', key_pos);
    if (value_pos == std::string::npos)
    {
      return default_value;
    }

    std::string token = Trim(text.substr(value_pos + 1));
    if (token.rfind("true", 0) == 0)
    {
      return true;
    }

    if (token.rfind("false", 0) == 0)
    {
      return false;
    }

    return default_value;
  }

  std::string JsonEscape(const std::string &value)
  {
    std::string escaped;
    escaped.reserve(value.size() + 16);
    for (const char ch : value)
    {
      switch (ch)
      {
      case '\\':
        escaped += "\\\\";
        break;
      case '"':
        escaped += "\\\"";
        break;
      case '\n':
        escaped += "\\n";
        break;
      case '\r':
        escaped += "\\r";
        break;
      case '\t':
        escaped += "\\t";
        break;
      default:
        escaped.push_back(ch);
        break;
      }
    }

    return escaped;
  }

  std::string Lowercase(std::string value)
  {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch)
                   { return static_cast<char>(std::tolower(ch)); });
    return value;
  }

  void ReplaceAll(std::string &text, const std::string &from, const std::string &to)
  {
    if (from.empty())
    {
      return;
    }

    std::size_t start = 0;
    while ((start = text.find(from, start)) != std::string::npos)
    {
      text.replace(start, from.length(), to);
      start += to.length();
    }
  }
} // namespace Editor
