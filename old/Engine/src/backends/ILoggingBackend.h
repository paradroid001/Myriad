#ifndef MYRIAD_BACKENDS_BACKEND_H
#define MYRIAD_BACKENDS_BACKEND_H

#include "core.h"

namespace Myriad
{
    class IBackend
    {
      public:
        virtual ~IBackend() {}
        // I don't even know what I need in a backend class
        virtual void Init() = 0;
        // virtual void Trace(... args);
    };
} // namespace Myriad
#endif
