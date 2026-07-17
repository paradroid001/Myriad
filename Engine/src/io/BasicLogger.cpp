#include "io/BasicLogger.h"

#include <cstdarg> // for va_list, va_start, va_end
#include <iostream>
#include <string>

namespace Myriad
{
    BasicLogger::BasicLogger(const std::string name)
        : name(name), log_level_(ILogger::MYR_LOGLEVEL_CRITICAL)
    {
    }
    BasicLogger::~BasicLogger()
    {
        Log(ILogger::MYR_LOGLEVEL_TRACE, "BasicLogger Destroyed.");
    }
    void BasicLogger::SetLogLevel(MyrLogLevel_t level) { log_level_ = level; }

    void BasicLogger::Log(MyrLogLevel_t level, const char *fmt, ...)
    {
        if (level > log_level_)
        {
            return;
        }

        std::va_list args;
        va_start(args, fmt);
        vsprintf(buf, fmt, args);
        std::cout << "[" << name << "] ";
        switch (level)
        {
        case ILogger::MYR_LOGLEVEL_TRACE:
            std::cout << REG_WHITE << "TRACE: ";
            break;
        case ILogger::MYR_LOGLEVEL_INFO:
            std::cout << REG_GREEN << "INFO: ";
            break;
        case ILogger::MYR_LOGLEVEL_WARNING:
            std::cout << REG_YELLOW << "WARNING: ";
            break;
        case ILogger::MYR_LOGLEVEL_ERROR:
            std::cout << REG_RED << "ERROR: ";
            break;
        case ILogger::MYR_LOGLEVEL_CRITICAL:
            std::cout << BOLD_RED << "CRITICAL: ";
            break;
        }
        std::cout << RESET << buf << std::endl;
        va_end(args);
    }
} // namespace Myriad
