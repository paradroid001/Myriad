#include "Log.h"
#include "spdlog/common.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace Myriad
{
    std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
    std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

    void Log::Init()
    {
        spdlog::set_pattern("%^[%T] %n: %v%$");
        s_CoreLogger = spdlog::stdout_color_mt("Core");
        s_CoreLogger->set_level(spdlog::level::trace);
        s_ClientLogger = spdlog::stdout_color_mt("APP");
        s_ClientLogger->set_level(spdlog::level::trace);
    }

    void Log::SetLogLevelTrace(std::shared_ptr<spdlog::logger> p_logger)
    {
        p_logger->set_level(spdlog::level::trace);
    }
    void Log::SetLogLevelWarn(std::shared_ptr<spdlog::logger> p_logger)
    {
        p_logger->set_level(spdlog::level::warn);
    }
    void Log::SetLogLevelInfo(std::shared_ptr<spdlog::logger> p_logger)
    {
        p_logger->set_level(spdlog::level::info);
    }
    void Log::SetLogLevelError(std::shared_ptr<spdlog::logger> p_logger)
    {
        p_logger->set_level(spdlog::level::err);
    }
        

} // namespace Myriad