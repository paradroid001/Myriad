#pragma once

#include <string>

namespace Editor
{
  /**
   * @brief Trims leading and trailing ASCII whitespace.
   * @param value Input string.
   * @return Trimmed string.
   */
  std::string Trim(const std::string &value);

  /**
   * @brief Extracts a string field from a JSON-like text blob.
   * @param text Source JSON text.
   * @param key Field name to look up.
   * @return Extracted value or empty string when not found.
   */
  std::string ExtractJsonString(const std::string &text, const std::string &key);

  /**
   * @brief Extracts an integer field from a JSON-like text blob.
   * @param text Source JSON text.
   * @param key Field name to look up.
   * @param default_value Fallback value when parsing fails.
   * @return Parsed integer or default_value.
   */
  int ExtractJsonInt(const std::string &text, const std::string &key, int default_value);

  /**
   * @brief Extracts a boolean field from a JSON-like text blob.
   * @param text Source JSON text.
   * @param key Field name to look up.
   * @param default_value Fallback value when parsing fails.
   * @return Parsed bool or default_value.
   */
  bool ExtractJsonBool(const std::string &text, const std::string &key, bool default_value);

  /**
   * @brief Escapes a string for safe insertion into JSON string literals.
   * @param value Raw input string.
   * @return Escaped string content (without surrounding quotes).
   */
  std::string JsonEscape(const std::string &value);

  /**
   * @brief Converts ASCII characters in a string to lowercase.
   * @param value Input string copied by value.
   * @return Lowercased string.
   */
  std::string Lowercase(std::string value);

  /**
   * @brief Replaces all occurrences of a substring in-place.
   * @param text Target string to modify.
   * @param from Substring to search for.
   * @param to Replacement substring.
   */
  void ReplaceAll(std::string &text, const std::string &from, const std::string &to);
} // namespace Editor
