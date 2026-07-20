#include "build_bridge.h"

#include "editor_settings.h"

#include <algorithm>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace Editor
{
#ifdef _WIN32
  using SocketHandle = SOCKET;
  constexpr SocketHandle InvalidSocketHandle = INVALID_SOCKET;

  void CloseSocketHandle(SocketHandle handle)
  {
    if (handle != InvalidSocketHandle)
    {
      closesocket(handle);
    }
  }

  bool EnsureWinsockStarted(std::string &error_message)
  {
    static bool attempted = false;
    static bool initialized = false;
    if (!attempted)
    {
      attempted = true;
      WSADATA data{};
      initialized = WSAStartup(MAKEWORD(2, 2), &data) == 0;
    }

    if (!initialized)
    {
      error_message = "WSAStartup failed.";
    }

    return initialized;
  }

  bool SetSocketTimeouts(SocketHandle handle, int timeout_ms)
  {
    const int timeout = timeout_ms;
    const int recv_result = setsockopt(handle, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char *>(&timeout), sizeof(timeout));
    const int send_result = setsockopt(handle, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char *>(&timeout), sizeof(timeout));
    return recv_result == 0 && send_result == 0;
  }
#else
  using SocketHandle = int;
  constexpr SocketHandle InvalidSocketHandle = -1;

  void CloseSocketHandle(SocketHandle handle)
  {
    if (handle != InvalidSocketHandle)
    {
      close(handle);
    }
  }

  bool SetSocketTimeouts(SocketHandle handle, int timeout_ms)
  {
    timeval timeout{};
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;
    const int recv_result = setsockopt(handle, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    const int send_result = setsockopt(handle, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    return recv_result == 0 && send_result == 0;
  }
#endif

  struct PersistentBridgeConnection
  {
    SocketHandle socket_handle = InvalidSocketHandle;
    std::string host;
    int port = 0;

    bool IsConnectedTo(const std::string &candidate_host, int candidate_port) const
    {
      return socket_handle != InvalidSocketHandle && host == candidate_host && port == candidate_port;
    }

    void Disconnect()
    {
      if (socket_handle != InvalidSocketHandle)
      {
        CloseSocketHandle(socket_handle);
        socket_handle = InvalidSocketHandle;
      }
      host.clear();
      port = 0;
    }

    ~PersistentBridgeConnection()
    {
      Disconnect();
    }
  };

  PersistentBridgeConnection g_build_bridge_connection;

  bool SendAll(SocketHandle handle, const char *data, std::size_t size)
  {
    std::size_t sent = 0;
    while (sent < size)
    {
#ifdef _WIN32
      const int result = ::send(handle, data + sent, static_cast<int>(size - sent), 0);
#else
      const ssize_t result = ::send(handle, data + sent, size - sent, 0);
#endif
      if (result <= 0)
      {
        return false;
      }

      sent += static_cast<std::size_t>(result);
    }

    return true;
  }

  bool ConnectSocketWithTimeout(SocketHandle socket_handle,
                                const sockaddr *address,
                                socklen_t address_length,
                                int timeout_ms,
                                std::string &error_message)
  {
#ifdef _WIN32
    u_long nonblocking_mode = 1;
    if (ioctlsocket(socket_handle, FIONBIO, &nonblocking_mode) != 0)
    {
      error_message = "Failed to configure non-blocking socket.";
      return false;
    }

    const int connect_result = ::connect(socket_handle, address, static_cast<int>(address_length));
    if (connect_result == SOCKET_ERROR)
    {
      const int last_error = WSAGetLastError();
      if (last_error != WSAEWOULDBLOCK && last_error != WSAEINPROGRESS && last_error != WSAEINVAL)
      {
        error_message = "Socket connect failed.";
        nonblocking_mode = 0;
        ioctlsocket(socket_handle, FIONBIO, &nonblocking_mode);
        return false;
      }
    }

    fd_set write_fds;
    FD_ZERO(&write_fds);
    FD_SET(socket_handle, &write_fds);

    timeval timeout{};
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;
    const int select_result = select(0, nullptr, &write_fds, nullptr, &timeout);
    if (select_result <= 0)
    {
      error_message = select_result == 0 ? "Socket connect timed out." : "Socket select failed during connect.";
      nonblocking_mode = 0;
      ioctlsocket(socket_handle, FIONBIO, &nonblocking_mode);
      return false;
    }

    int so_error = 0;
    int opt_len = sizeof(so_error);
    if (getsockopt(socket_handle, SOL_SOCKET, SO_ERROR, reinterpret_cast<char *>(&so_error), &opt_len) != 0 || so_error != 0)
    {
      error_message = "Socket connect failed after select.";
      nonblocking_mode = 0;
      ioctlsocket(socket_handle, FIONBIO, &nonblocking_mode);
      return false;
    }

    nonblocking_mode = 0;
    ioctlsocket(socket_handle, FIONBIO, &nonblocking_mode);
    return true;
#else
    const int existing_flags = fcntl(socket_handle, F_GETFL, 0);
    if (existing_flags < 0 || fcntl(socket_handle, F_SETFL, existing_flags | O_NONBLOCK) < 0)
    {
      error_message = "Failed to configure non-blocking socket.";
      return false;
    }

    const int connect_result = ::connect(socket_handle, address, address_length);
    if (connect_result != 0 && errno != EINPROGRESS)
    {
      error_message = "Socket connect failed.";
      fcntl(socket_handle, F_SETFL, existing_flags);
      return false;
    }

    fd_set write_fds;
    FD_ZERO(&write_fds);
    FD_SET(socket_handle, &write_fds);

    timeval timeout{};
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;
    const int select_result = select(socket_handle + 1, nullptr, &write_fds, nullptr, &timeout);
    if (select_result <= 0)
    {
      error_message = select_result == 0 ? "Socket connect timed out." : "Socket select failed during connect.";
      fcntl(socket_handle, F_SETFL, existing_flags);
      return false;
    }

    int so_error = 0;
    socklen_t opt_len = sizeof(so_error);
    if (getsockopt(socket_handle, SOL_SOCKET, SO_ERROR, &so_error, &opt_len) != 0 || so_error != 0)
    {
      error_message = "Socket connect failed after select.";
      fcntl(socket_handle, F_SETFL, existing_flags);
      return false;
    }

    fcntl(socket_handle, F_SETFL, existing_flags);
    return true;
#endif
  }

  bool ConnectToBuildBridge(const std::string &host, int port, SocketHandle &socket_handle, std::string &error_message, int connect_timeout_ms)
  {
    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo *results = nullptr;
    const std::string port_text = std::to_string(port);
    if (const int result = getaddrinfo(host.c_str(), port_text.c_str(), &hints, &results); result != 0)
    {
#ifdef _WIN32
      error_message = std::string{"getaddrinfo failed: "} + gai_strerrorA(result);
#else
      error_message = std::string{"getaddrinfo failed: "} + gai_strerror(result);
#endif
      return false;
    }

    bool connected = false;
    for (addrinfo *entry = results; entry != nullptr; entry = entry->ai_next)
    {
      socket_handle = static_cast<SocketHandle>(::socket(entry->ai_family, entry->ai_socktype, entry->ai_protocol));
      if (socket_handle == InvalidSocketHandle)
      {
        continue;
      }

      if (ConnectSocketWithTimeout(socket_handle,
                                   entry->ai_addr,
                                   static_cast<socklen_t>(entry->ai_addrlen),
                                   connect_timeout_ms,
                                   error_message))
      {
        connected = true;
        break;
      }

      CloseSocketHandle(socket_handle);
      socket_handle = InvalidSocketHandle;
    }

    freeaddrinfo(results);

    if (!connected)
    {
      error_message = "Unable to connect to build bridge host.";
      return false;
    }

    return true;
  }

  bool EnsurePersistentBridgeConnected(PersistentBridgeConnection &connection,
                                       const std::string &host,
                                       int port,
                                       int connect_timeout_ms,
                                       std::string &error_message)
  {
    if (connection.IsConnectedTo(host, port))
    {
      return true;
    }

    connection.Disconnect();

#ifdef _WIN32
    if (!EnsureWinsockStarted(error_message))
    {
      return false;
    }
#endif

    SocketHandle socket_handle = InvalidSocketHandle;
    if (!ConnectToBuildBridge(host, port, socket_handle, error_message, connect_timeout_ms))
    {
      return false;
    }

    connection.socket_handle = socket_handle;
    connection.host = host;
    connection.port = port;
    return true;
  }

  bool SendBuildBridgeRequestPersistent(PersistentBridgeConnection &connection,
                                        const std::string &host,
                                        int port,
                                        const std::string &request_text,
                                        std::string &response_text,
                                        std::string &error_message,
                                        const std::function<void(const std::string &)> &on_console_line,
                                        int connect_timeout_ms,
                                        int io_timeout_ms)
  {
    auto try_once = [&]() -> bool
    {
      if (!EnsurePersistentBridgeConnected(connection, host, port, connect_timeout_ms, error_message))
      {
        return false;
      }

      if (!SetSocketTimeouts(connection.socket_handle, io_timeout_ms))
      {
        error_message = "Failed to configure build bridge socket timeouts.";
        connection.Disconnect();
        return false;
      }

      if (!SendAll(connection.socket_handle, request_text.c_str(), request_text.size()))
      {
        error_message = "Failed to send build request.";
        connection.Disconnect();
        return false;
      }

      char buffer[4096];
      std::string pending_line;
      std::string final_response;
      response_text.clear();

      while (final_response.empty())
      {
#ifdef _WIN32
        const int received = ::recv(connection.socket_handle, buffer, sizeof(buffer), 0);
#else
        const ssize_t received = ::recv(connection.socket_handle, buffer, sizeof(buffer), 0);
#endif
        if (received < 0)
        {
          error_message = "Failed to read build response.";
          connection.Disconnect();
          return false;
        }

        if (received == 0)
        {
          error_message = "Build bridge closed the connection.";
          connection.Disconnect();
          return false;
        }

        pending_line.append(buffer, static_cast<std::size_t>(received));
        std::size_t line_start = 0;
        while (true)
        {
          const std::size_t line_end = pending_line.find('\n', line_start);
          if (line_end == std::string::npos)
          {
            pending_line.erase(0, line_start);
            break;
          }

          std::string line = pending_line.substr(line_start, line_end - line_start);
          if (!line.empty() && line.back() == '\r')
          {
            line.pop_back();
          }

          constexpr const char *log_prefix = "@@MYRIAD-BRIDGE-LOG@@ ";
          constexpr const char *response_prefix = "@@MYRIAD-BRIDGE-RESPONSE@@ ";
          if (line.rfind(log_prefix, 0) == 0)
          {
            if (on_console_line)
            {
              on_console_line(line.substr(std::char_traits<char>::length(log_prefix)));
            }
          }
          else if (line.rfind(response_prefix, 0) == 0)
          {
            final_response = line.substr(std::char_traits<char>::length(response_prefix));
            break;
          }
          else if (!line.empty())
          {
            response_text.append(line);
            response_text.push_back('\n');
          }

          line_start = line_end + 1;
        }
      }

      if (!final_response.empty())
      {
        response_text = std::move(final_response);
      }

      if (response_text.empty())
      {
        error_message = "Build bridge returned no response.";
        return false;
      }

      return true;
    };

    if (try_once())
    {
      return true;
    }

    if (connection.socket_handle == InvalidSocketHandle)
    {
      if (try_once())
      {
        return true;
      }
    }

    return false;
  }

  void DisconnectBuildBridge()
  {
    g_build_bridge_connection.Disconnect();
  }

  bool SendBuildBridgeRequestWithFallback(EditorSettings &settings,
                                          const std::string &request_text,
                                          std::string &response_text,
                                          std::string &error_message,
                                          const std::function<void(const std::string &)> &on_console_line,
                                          int connect_timeout_ms,
                                          int io_timeout_ms)
  {
    std::vector<std::string> attempt_errors;
    std::string tried_hosts_text;
    for (const auto &host : BuildBridgeHostCandidates(settings))
    {
      if (!tried_hosts_text.empty())
      {
        tried_hosts_text += ", ";
      }
      tried_hosts_text += host;

      if (SendBuildBridgeRequestPersistent(g_build_bridge_connection,
                                           host,
                                           settings.build_socket_port,
                                           request_text,
                                           response_text,
                                           error_message,
                                           on_console_line,
                                           connect_timeout_ms,
                                           io_timeout_ms))
      {
        settings.build_socket_host = host;
        return true;
      }

      if (!error_message.empty())
      {
        attempt_errors.push_back(host + ": " + error_message);
      }
      else
      {
        attempt_errors.push_back(host + ": Unknown socket error");
      }
    }

    if (attempt_errors.empty())
    {
      error_message = "Unable to connect to build bridge host.";
    }
    else
    {
      std::ostringstream combined;
      combined << "Unable to connect to build bridge host. Hosts tried: " << tried_hosts_text << ".";
      combined << " Errors: ";
      for (std::size_t i = 0; i < attempt_errors.size(); ++i)
      {
        if (i > 0)
        {
          combined << " | ";
        }
        combined << attempt_errors[i];
      }
      error_message = combined.str();
    }

    return false;
  }

  bool ProbeBuildBridge(EditorSettings &settings, std::string &response_text, std::string &error_message)
  {
    const std::string request = CreateBuildBridgeStatusRequest(settings.project_mount_path, settings.target_executable_name, settings.source_directory);
    return SendBuildBridgeRequestWithFallback(settings, request, response_text, error_message, nullptr, 250, 800);
  }
} // namespace Editor
