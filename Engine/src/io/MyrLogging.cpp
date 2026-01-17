#include "io/MyrLogging.h"
#include "io/ILogger.h"
#include <iostream>
#include <string>
#include <vector>

#include "spdlog/common.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace Myriad
{
    MyrLogging *MyrLogging::s_logging_ptr_;

    MyrLogging::MyrLogging()
    {

        std::vector<spdlog::sink_ptr> core_sinks;
        std::vector<spdlog::sink_ptr> client_sinks;
        core_sinks.push_back(
            std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        client_sinks.push_back(
            std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

        core_logger = std::make_shared<spdlog::logger>(
            "Core", core_sinks.begin(), core_sinks.end());
        client_logger = std::make_shared<spdlog::logger>(
            "Client", client_sinks.begin(), client_sinks.end());

        s_logging_ptr_ = this;
        SetCoreLogLevel(MyrLogLevel_t::MYR_LOGLEVEL_TRACE);
        SetClientLogLevel(MyrLogLevel_t::MYR_LOGLEVEL_TRACE);
    }
    MyrLogging::~MyrLogging()
    {
        std::cout << "MyrLogging destructor" << std::endl;
    }

    // This is a static function
    void MyrLogging::SetClientLogLevel(MyrLogLevel_t logLevel)
    {
        s_logging_ptr_->SetLogLevel(s_logging_ptr_->client_logger, logLevel);
    }
    void MyrLogging::SetCoreLogLevel(MyrLogLevel_t logLevel)
    {
        s_logging_ptr_->SetLogLevel(s_logging_ptr_->core_logger, logLevel);
    }

    void MyrLogging::SetLogLevel(std::shared_ptr<spdlog::logger> &spdlog_ptr,
                                 MyrLogLevel_t logLevel)
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
} // namespace Myriad
