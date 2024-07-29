#ifndef MYRIAD_CORE_ALLOCATORPROVIDER_H
#define MYRIAD_CORE_ALLOCATORPROVIDER_H

#include "core/MyrHandle.h"
#include "core/Provider.h"
#include "io/Log.h"
#include <cassert>
#include <iostream>
#include <map>

namespace Myriad
{
    class AllocatorProvider : public Provider
    {
      public:
        virtual bool Init() = 0;
        virtual bool Shutdown() = 0;
        // So here, I C++ doesn't support virtual variadic functions...
        template <class T, typename... Args> uint16_t Alloc(Args... args)
        {
            return 0; /*override me*/
        };
        template <class T> T *Get(uint16_t index)
        {
            return NULL; /*override me*/
        };
    };
} // namespace Myriad

#endif
