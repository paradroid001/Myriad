#pragma once

#include "editor_types.h"

#include <functional>
#include <string>

namespace Editor
{
  /**
   * @brief Closes any cached build-bridge socket connection.
   */
  void DisconnectBuildBridge();

  /**
   * @brief Sends a JSON request to the build bridge with host fallback.
   * @param settings Editor settings used for host/port lookup and host caching.
   * @param request_text Serialized request payload terminated by a newline.
   * @param response_text Receives the final JSON response from the bridge.
   * @param error_message Receives a human-readable error when the request fails.
   * @param on_console_line Callback invoked for streamed bridge log lines.
   * @param connect_timeout_ms Per-host TCP connect timeout in milliseconds.
   * @param io_timeout_ms Socket I/O timeout in milliseconds.
   * @return true if a response was received; otherwise false.
   */
  bool SendBuildBridgeRequestWithFallback(EditorSettings &settings,
                                          const std::string &request_text,
                                          std::string &response_text,
                                          std::string &error_message,
                                          const std::function<void(const std::string &)> &on_console_line,
                                          int connect_timeout_ms,
                                          int io_timeout_ms);

  /**
   * @brief Probes bridge reachability using a status request.
   * @param settings Editor settings used for endpoint resolution.
   * @param response_text Receives the JSON status payload on success.
   * @param error_message Receives an error message on failure.
   * @return true when the bridge responded successfully.
   */
  bool ProbeBuildBridge(EditorSettings &settings, std::string &response_text, std::string &error_message);
} // namespace Editor
