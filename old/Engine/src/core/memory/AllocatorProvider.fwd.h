#ifndef MYRIAD_CORE_MEMORY_ALLOCATORPROVIDERFWD_H
#define MYRIAD_CORE_MEMORY_ALLOCATORPROVIDERFWD_H

// #include "core/Provider.h"
#include "core/core.h"
namespace Myriad
{
    // A Forward declaration to satisfy the dependence between MyrHandle and
    // AllocatorProvider
    template <class T> class MYR_API AllocatorProvider;
} // namespace Myriad
#endif
