#include "io/MyrLogging.h"
#include <string>
#include <vector>
#include <iostream>
namespace Myriad
{
  /*
  MyrLogger::MyrLogger()
  {
  }

  MyrLogger::~MyrLogger()
  {
  }

  bool MyrLogger::AddLogger(std::string name, ILogger *pLogger)
  {
    bool ret = false;
    if (loggers_map_.find(name) != loggers_map_.end())
    {
      // key already exists. Log that. Return false.
      Log(MyrLogLevel_t::MYR_LOGLEVEL_WARNING, "Could not add Logger {0}, key already existed", name.c_str());
    }
    else
    {
      // Add the key.
      loggers_map_[name] = pLogger;
      Log(MyrLogLevel_t::MYR_LOGLEVEL_INFO, "Added logger {0}, there are now {1} loggers", name.c_str(), loggers_map_.size());
    }
    return ret;
  }
  bool MyrLogger::RemoveLogger(std::string name)
  {
    return false;
  }
  void MyrLogger::Log(MyrLogLevel_t level, const char *fmt, ...)
  {
    for (auto it = loggers_map_.begin(); it != loggers_map_.end(); ++it)
    {
      it->second->Log(level, fmt, ...);
    }
  }
  */
  std::shared_ptr<spdlog::logger> MyrLogging::s_client_logger_;
  std::shared_ptr<spdlog::logger> MyrLogging::s_core_logger_;
  MyrLogging *MyrLogging::s_logging_ptr_;

  MyrLogging::MyrLogging()
  {
    std::vector<spdlog::sink_ptr> core_sinks;
    std::vector<spdlog::sink_ptr> client_sinks;
    core_sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    client_sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

    s_core_logger_ = std::make_shared<spdlog::logger>("Core", core_sinks.begin(), core_sinks.end());
    s_client_logger_ = std::make_shared<spdlog::logger>("Client", client_sinks.begin(), client_sinks.end());
    s_logging_ptr_ = this;
    SetCoreLogLevel(MyrLogLevel_t::MYR_LOGLEVEL_TRACE);
    SetClientLogLevel(MyrLogLevel_t::MYR_LOGLEVEL_TRACE);
  }
  MyrLogging::~MyrLogging()
  {
    std::cout << "MyrLogging destructor" << std::endl;
  }
  void MyrLogging::AddCoreLogger(std::string name, ILogger *pLogger)
  {
    // core_logger_.AddLogger(name, pLogger);
    // core_logger_->sinks().push_back
  }
  void MyrLogging::RemoveCoreLogger(std::string name)
  {
    // core_logger_.RemoveLogger(name);
  }
  void MyrLogging::AddClientLogger(std::string name, ILogger *pLogger)
  {
    // client_logger_.AddLogger(name, pLogger);
  }
  void MyrLogging::RemoveClientLogger(std::string name)
  {
    // client_logger_.RemoveLogger(name);
  }

  void MyrLogging::SetClientLogLevel(MyrLogLevel_t logLevel)
  {
    SetLogLevel(GetClientLogger(), logLevel);
  }
  void MyrLogging::SetCoreLogLevel(MyrLogLevel_t logLevel)
  {
    SetLogLevel(GetCoreLogger(), logLevel);
  }

  void MyrLogging::SetLogLevel(std::shared_ptr<spdlog::logger> &spdlog_ptr, MyrLogLevel_t logLevel)
  {
    std::shared_ptr<spdlog::logger> p_spdlog_ = spdlog_ptr;

    if (p_spdlog_ != nullptr)
    {
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
}
