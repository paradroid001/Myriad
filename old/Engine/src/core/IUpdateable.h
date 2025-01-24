#ifndef MYRIAD_CORE_IUPDATEABLE_H
#define MYRIAD_CORE_IUPDATEABLE_H

#include "core/core.h"

namespace Myriad
{
    class MYR_API IUpdateable
    {
      public:
        virtual void Update(float dt) = 0;
    };
} // namespace Myriad

#endif
