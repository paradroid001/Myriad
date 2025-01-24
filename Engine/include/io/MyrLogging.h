#ifndef MYRIAD_IO_MYRLOGGING_H
#define MYRIAD_IO_MYRLOGGING_H

#include "core/core.h"
#include "io/ILogger.h"
#include <unordered_map>
#include <string>
#include <cstdarg>

#include "spdlog/spdlog.h"
#include "spdlog/common.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Myriad
{
  /*
  class MyrLogger : ILogger
  {
  private:
    std::unordered_map<std::string, ILogger *> loggers_map_;

  public:
    MyrLogger();
    ~MyrLogger();
    bool AddLogger(std::string name, ILogger *pLogger);
    bool RemoveLogger(std::string name);
    // ILogger
    virtual void Log(MyrLogLevel_t level, const char *fmt, ...) override;
  };
  */
  class MYR_API MyrLogging
  {
  private:
    // MyrLogger core_logger_;
    // MyrLogger client_logger_;
    static std::shared_ptr<spdlog::logger> s_core_logger_;
    static std::shared_ptr<spdlog::logger> s_client_logger_;
    static MyrLogging *s_logging_ptr_;

  public:
    MyrLogging();
    ~MyrLogging();
    void AddCoreLogger(std::string name, ILogger *pLogger);
    void RemoveCoreLogger(std::string name);
    void AddClientLogger(std::string name, ILogger *pLogger);
    void RemoveClientLogger(std::string name);

    void SetClientLogLevel(MyrLogLevel_t logLevel);
    void SetCoreLogLevel(MyrLogLevel_t logLevel);

    void SetLogLevel(std::shared_ptr<spdlog::logger> &spdlog_ptr, MyrLogLevel_t logLevel);
    static inline std::shared_ptr<spdlog::logger> &GetClientLogger() { return s_client_logger_; }
    static inline std::shared_ptr<spdlog::logger> &GetCoreLogger() { return s_core_logger_; }
    static inline MyrLogging *GetLogging() { return s_logging_ptr_; }
    // Template functions must be defined in the header
    template <class... Args>
    void Log(std::shared_ptr<spdlog::logger> &spdlog_ptr, MyrLogLevel_t logLevel, const char *fmt, Args... args)
    {
      // core_logger_.Log(logLevel, fmt, args...);
      switch (logLevel)
      {
      case MyrLogLevel_t::MYR_LOGLEVEL_TRACE:
      {
        spdlog_ptr->trace(fmt, args...);
        break;
      }
      case MyrLogLevel_t::MYR_LOGLEVEL_INFO:
      {
        spdlog_ptr->info(fmt, args...);
        break;
      }
      case MyrLogLevel_t::MYR_LOGLEVEL_WARNING:
      {
        spdlog_ptr->warn(fmt, args...);
        break;
      }
      case MyrLogLevel_t::MYR_LOGLEVEL_ERROR:
      {
        spdlog_ptr->error(fmt, args...);
        break;
      }
      case MyrLogLevel_t::MYR_LOGLEVEL_CRITICAL:
      {
        spdlog_ptr->error(fmt, args...);
        break;
      }
      }
    }
  };

#define MYR_CORE_TRACE(...) ::Myriad::MyrLogging::GetLogging()->Log(::Myriad::MyrLogging::GetCoreLogger(), Myriad::MyrLogLevel_t::MYR_LOGLEVEL_TRACE, __VA_ARGS__)
#define MYR_CORE_INFO(...) ::Myriad::MyrLogging::GetLogging()->Log(::Myriad::MyrLogging::GetCoreLogger(), Myriad::MyrLogLevel_t::MYR_LOGLEVEL_INFO, __VA_ARGS__)
#define MYR_CORE_WARN(...) ::Myriad::MyrLogging::GetLogging()->Log(::Myriad::MyrLogging::GetCoreLogger(), Myriad::MyrLogLevel_t::MYR_LOGLEVEL_WARNING, __VA_ARGS__)
#define MYR_CORE_ERROR(...) ::Myriad::MyrLogging::GetLogging()->Log(::Myriad::MyrLogging::GetCoreLogger(), Myriad::MyrLogLevel_t::MYR_LOGLEVEL_ERROR, __VA_ARGS__)

#define MYR_TRACE(...) ::Myriad::MyrLogging::GetLogging()->Log(::Myriad::MyrLogging::GetClientLogger(), Myriad::MyrLogLevel_t::MYR_LOGLEVEL_TRACE, __VA_ARGS__)
#define MYR_INFO(...) ::Myriad::MyrLogging::GetLogging()->Log(::Myriad::MyrLogging::GetClientLogger(), Myriad::MyrLogLevel_t::MYR_LOGLEVEL_INFO, __VA_ARGS__)
#define MYR_WARN(...) ::Myriad::MyrLogging::GetLogging()->Log(::Myriad::MyrLogging::GetClientLogger(), Myriad::MyrLogLevel_t::MYR_LOGLEVEL_WARNING, __VA_ARGS__)
#define MYR_ERROR(...) ::Myriad::MyrLogging::GetLogging()->Log(::Myriad::MyrLogging::GetClientLogger(), Myriad::MyrLogLevel_t::MYR_LOGLEVEL_ERROR, __VA_ARGS__)

}
#endif
