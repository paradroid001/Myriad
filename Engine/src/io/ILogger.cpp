#ifndef _MYRIAD_IO_ILOGGER_H_
#define _MYRIAD_IO_ILOGGER_H_

#include "myriad.h" //ILogger and global loggers.

#include "io/Logging.h" // Core Engine Logging Macros

namespace Myriad
{
    // The pure virtual destructor from myriad.h needs
    // to be defined here.
    ILogger::~ILogger() {}
} // namespace Myriad

#endif
