#ifndef MYRIAD_IO_ILOGGER
#define MYRIAD_IO_ILOGGER

#include "core/core.h"
#include <cstdarg> //for variadic functions

namespace Myriad
{
    typedef enum MyrLogLevel_t
    {
        MYR_LOGLEVEL_TRACE = 0,
        MYR_LOGLEVEL_INFO = 2,
        MYR_LOGLEVEL_WARNING = 4,
        MYR_LOGLEVEL_ERROR = 8,
        MYR_LOGLEVEL_CRITICAL = 16,
    } MyrLogLevel_t;

    class MYR_API ILogger
    {
      protected:
        MyrLogLevel_t log_level_;

      public:
        inline virtual void SetLogLevel(MyrLogLevel_t logLevel)
        {
            log_level_ = logLevel;
        }
        virtual void Log(MyrLogLevel_t loglevel, const char *fmt, ...) = 0;
    };
} // namespace Myriad
#endif
