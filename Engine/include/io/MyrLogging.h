#ifndef MYRIAD_IO_MYRLOGGING_H
#define MYRIAD_IO_MYRLOGGING_H

#include "core/core.h"
#include "io/ILogger.h"
#include <memory> //shared_ptr

// Unfortunately we can't get away from including these,
// as the logging macros will take ... and the function template
// which handles Args... needs to be in the header file, and this
// needs to call spdlog->x functions because you can't make
// virtual function templates - so I can't subclass.
//
// This means the spdlog includes need to be shipped with myriad dll/so

#include "spdlog/spdlog.h"

namespace Myriad
{
    class MYR_API MyrLogging
    {
      private:
        // The client and core loggers
        std::shared_ptr<spdlog::logger> core_logger;
        std::shared_ptr<spdlog::logger> client_logger;

        // Static global pointer to ourselves returned by
        // MyrLogging::GetLogging()
        static MyrLogging *s_logging_ptr_;

        void SetLogLevel(std::shared_ptr<spdlog::logger> &spdlog_ptr,
                         MyrLogLevel_t logLevel);

      public:
        MyrLogging();
        ~MyrLogging();

        static void SetClientLogLevel(MyrLogLevel_t logLevel);
        static void SetCoreLogLevel(MyrLogLevel_t logLevel);

        static inline std::shared_ptr<spdlog::logger> &GetClientLogger()
        {
            return s_logging_ptr_->client_logger;
        }
        static inline std::shared_ptr<spdlog::logger> &GetCoreLogger()
        {
            return s_logging_ptr_->core_logger;
        }

        static inline MyrLogging *GetLogging() { return s_logging_ptr_; }

        // Template functions must be defined in the header
        template <class... Args>
        void Log(std::shared_ptr<spdlog::logger> &spdlog_ptr,
                 MyrLogLevel_t logLevel, const char *fmt, Args... args)
        {
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

#define MYR_CORE_TRACE(...)                                                    \
    ::Myriad::MyrLogging::GetLogging()->Log(                                   \
        ::Myriad::MyrLogging::GetCoreLogger(),                                 \
        Myriad::MyrLogLevel_t::MYR_LOGLEVEL_TRACE, __VA_ARGS__)
#define MYR_CORE_INFO(...)                                                     \
    ::Myriad::MyrLogging::GetLogging()->Log(                                   \
        ::Myriad::MyrLogging::GetCoreLogger(),                                 \
        Myriad::MyrLogLevel_t::MYR_LOGLEVEL_INFO, __VA_ARGS__)
#define MYR_CORE_WARN(...)                                                     \
    ::Myriad::MyrLogging::GetLogging()->Log(                                   \
        ::Myriad::MyrLogging::GetCoreLogger(),                                 \
        Myriad::MyrLogLevel_t::MYR_LOGLEVEL_WARNING, __VA_ARGS__)
#define MYR_CORE_ERROR(...)                                                    \
    ::Myriad::MyrLogging::GetLogging()->Log(                                   \
        ::Myriad::MyrLogging::GetCoreLogger(),                                 \
        Myriad::MyrLogLevel_t::MYR_LOGLEVEL_ERROR, __VA_ARGS__)

#define MYR_TRACE(...)                                                         \
    ::Myriad::MyrLogging::GetLogging()->Log(                                   \
        ::Myriad::MyrLogging::GetClientLogger(),                               \
        Myriad::MyrLogLevel_t::MYR_LOGLEVEL_TRACE, __VA_ARGS__)
#define MYR_INFO(...)                                                          \
    ::Myriad::MyrLogging::GetLogging()->Log(                                   \
        ::Myriad::MyrLogging::GetClientLogger(),                               \
        Myriad::MyrLogLevel_t::MYR_LOGLEVEL_INFO, __VA_ARGS__)
#define MYR_WARN(...)                                                          \
    ::Myriad::MyrLogging::GetLogging()->Log(                                   \
        ::Myriad::MyrLogging::GetClientLogger(),                               \
        Myriad::MyrLogLevel_t::MYR_LOGLEVEL_WARNING, __VA_ARGS__)
#define MYR_ERROR(...)                                                         \
    ::Myriad::MyrLogging::GetLogging()->Log(                                   \
        ::Myriad::MyrLogging::GetClientLogger(),                               \
        Myriad::MyrLogLevel_t::MYR_LOGLEVEL_ERROR, __VA_ARGS__)

} // namespace Myriad
#endif
