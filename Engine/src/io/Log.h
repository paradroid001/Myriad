#ifndef __LOG_H_
#define __LOG_H_

#include <memory>

#include "core/core.h"

#if defined(_WIN32)
    #define NOGDI  // All GDI defines and routines
    #define NOUSER // All USER defines and routines
#endif

#include "spdlog/spdlog.h"

namespace Myriad
{
    // class MYR_API Logger
    //{
    //     template <typename... Args>
    //     virtual void trace(format_string_t<Args...> fmt, Args &&...args) = 0;
    // }

    class MYR_API Log
    {
      public:
        static void Init();
        inline static std::shared_ptr<spdlog::logger> &GetCoreLogger()
        {
            return s_CoreLogger;
        }
        inline static std::shared_ptr<spdlog::logger> &GetClientLogger()
        {
            return s_ClientLogger;
        }
        static void SetLogLevelTrace(std::shared_ptr<spdlog::logger> p_logger);
        static void SetLogLevelWarn(std::shared_ptr<spdlog::logger> p_logger);
        static void SetLogLevelInfo(std::shared_ptr<spdlog::logger> p_logger);
        static void SetLogLevelError(std::shared_ptr<spdlog::logger> p_logger);
        

      private:
        static std::shared_ptr<spdlog::logger> s_CoreLogger;
        static std::shared_ptr<spdlog::logger> s_ClientLogger;
    };
} // namespace Myriad

#define MYR_CORE_TRACE(...) ::Myriad::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define MYR_CORE_WARN(...) ::Myriad::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define MYR_CORE_INFO(...) ::Myriad::Log::GetCoreLogger()->info(__VA_ARGS__)
#define MYR_CORE_ERROR(...) ::Myriad::Log::GetCoreLogger()->error(__VA_ARGS__)

#define MYR_TRACE(...) ::Myriad::Log::GetClientLogger()->trace(__VA_ARGS__)
#define MYR_WARN(...) ::Myriad::Log::GetClientLogger()->warn(__VA_ARGS__)
#define MYR_INFO(...) ::Myriad::Log::GetClientLogger()->info(__VA_ARGS__)
#define MYR_ERROR(...) ::Myriad::Log::GetClientLogger()->error(__VA_ARGS__)

#endif