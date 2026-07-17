#ifndef _MYRIAD_IO_LOGGING_H_
#define _MYRIAD_IO_LOGGING_H_

#include "myriad.h" //ILogger and global loggers.

#define MYR_CORE_TRACE(...)                                                    \
    global_logger_core->Log(ILogger::MyrLogLevel_t::MYR_LOGLEVEL_TRACE,        \
                            __VA_ARGS__)
#define MYR_CORE_INFO(...)                                                     \
    global_logger_core->Log(ILogger::MyrLogLevel_t::MYR_LOGLEVEL_INFO,         \
                            __VA_ARGS__)
#define MYR_CORE_WARN(...)                                                     \
    global_logger_core->Log(ILogger::MyrLogLevel_t::MYR_LOGLEVEL_WARNING,      \
                            __VA_ARGS__)
#define MYR_CORE_ERROR(...)                                                    \
    global_logger_core->Log(ILogger::MyrLogLevel_t::MYR_LOGLEVEL_ERROR,        \
                            __VA_ARGS__)
#define MYR_CORE_CRITICAL(...)                                                 \
    global_logger_core->Log(ILogger::MyrLogLevel_t::MYR_LOGLEVEL_CRITICAL,     \
                            __VA_ARGS__)

#endif
