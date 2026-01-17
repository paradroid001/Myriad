#ifndef MYRIAD_IO_LOGGERSPDLOG_H
#define MYRIAD_IO_LOGGERSPDLOG_H

#include <memory>

#include "core/core.h"
#include "io/ILogger.h"

// For all the spdlog implementation.
#include "spdlog/common.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace Myriad
{
    class MYR_API LoggerSpdLog : public ILogger
    {
      protected:
        std::shared_ptr<spdlog::logger> p_spdlog_;

      public:
        LoggerSpdLog();
        virtual ~LoggerSpdLog();
        void SetLogLevel(MyrLogLevel_t logLevel) override;
        virtual void Log(MyrLogLevel_t logLevel, const char *fmt, ...) override;
    };
} // namespace Myriad
#endif
