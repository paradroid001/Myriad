#ifndef _MYRIAD_IO_BASICLOGGER_H_
#define _MYRIAD_IO_BASICLOGGER_H_

#include "myriad.h"

#include <string>

#include "io/Logging.h"

// Regular Colors
#define REG_RED "\033[0;31m"
#define REG_GREEN "\033[0;32m"
#define REG_YELLOW "\033[0;33m"
#define REG_BLUE "\033[0;34m"
#define REG_WHITE "\033[0;37m"

// Bold Colors
#define BOLD_RED "\033[1;31m"
#define BOLD_GREEN "\033[1;32m"
#define BOLD_YELLOW "\033[1;33m"
#define BOLD_BLUE "\033[1;34m"

// Background Colors
#define BG_RED "\033[41m"
#define BG_GREEN "\033[42m"

// Text Reset
#define RESET "\033[0m"

namespace Myriad
{
    class BasicLogger : public ILogger
    {
      private:
        char buf[1024]; // buffer for formatted log messages
        std::string name;

      protected:
        MyrLogLevel_t log_level_;

      public:
        BasicLogger(const std::string name);
        virtual ~BasicLogger();
        virtual void SetLogLevel(MyrLogLevel_t level) override;
        virtual void Log(MyrLogLevel_t level, const char *fmt, ...) override;
    };
} // namespace Myriad

#endif
