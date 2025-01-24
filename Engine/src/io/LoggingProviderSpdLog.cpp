#include <iostream>

#include "io/LoggingProviderSpdLog.h"
#include "spdlog/common.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace Myriad
{
  LoggerSpdLog::LoggerSpdLog() : ILogger()
  {
    spdlog::set_pattern("%^[%T] %n: %v%$");
    p_spdlog_ = spdlog::stdout_color_mt("spdlog");
    p_spdlog_->set_level(spdlog::level::trace); // just an init level
  };
  LoggerSpdLog::~LoggerSpdLog()
  {
    std::cout << "LoggingProviderSpdLog destructor" << std::endl;
  };

  void LoggerSpdLog::SetLogLevel(MyrLogLevel_t logLevel)
  {
    if (p_spdlog_ != nullptr)
    {
      log_level_ = logLevel;
      switch (logLevel)
      {
      case MyrLogLevel_t::MYR_LOGLEVEL_TRACE:
        p_spdlog_->set_level(spdlog::level::trace);
        break;
      case MyrLogLevel_t::MYR_LOGLEVEL_INFO:
        p_spdlog_->set_level(spdlog::level::info);
        break;
      case MyrLogLevel_t::MYR_LOGLEVEL_WARNING:
        p_spdlog_->set_level(spdlog::level::warn);
        break;
      case MyrLogLevel_t::MYR_LOGLEVEL_ERROR:
        p_spdlog_->set_level(spdlog::level::err);
        break;
      case MyrLogLevel_t::MYR_LOGLEVEL_CRITICAL:
        p_spdlog_->set_level(spdlog::level::err);
        break;
      }
    }
  }
  void LoggerSpdLog::Log(MyrLogLevel_t logLevel, const char *fmt, ...)
  {
    switch (log_level_)
    {
    case MyrLogLevel_t::MYR_LOGLEVEL_TRACE:
    {
      p_spdlog_->trace(fmt);
      break;
    }
    case MyrLogLevel_t::MYR_LOGLEVEL_INFO:
    {
      p_spdlog_->info(fmt);
      break;
    }
    case MyrLogLevel_t::MYR_LOGLEVEL_WARNING:
    {
      p_spdlog_->warn(fmt);
      break;
    }
    case MyrLogLevel_t::MYR_LOGLEVEL_ERROR:
    {
      p_spdlog_->error(fmt);
      break;
    }
    case MyrLogLevel_t::MYR_LOGLEVEL_CRITICAL:
    {
      p_spdlog_->error(fmt);
      break;
    }
    }
    std::cout << "spdlog output" << std::endl;
  }
}
