#ifndef MYRIAD_CORE_MEMORY_MYRHANDLE_FWD_H
#define MYRIAD_CORE_MEMORY_MYRHANDLE_FWD_H

#include "core/core.h"
#include <cstdint>
namespace Myriad
{
    // MyrHandle 'light' is a uint16
    typedef uint16_t MyrHandle_T;
    template <class T> class MYR_API MyrHandle;
} // namespace Myriad
#endif
