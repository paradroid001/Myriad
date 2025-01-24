#ifndef MYRIAD_CORE_OBJECT_IENABLEABLE_H
#define MYRIAD_CORE_OBJECT_IENABLEABLE_H

#include "core/core.h"

namespace Myriad
{
    class MYR_API IEnableable
    {
      public:
        // TODO does this interface really need destructor?
        virtual ~IEnableable() {}
        virtual void Enable() = 0;
        virtual void Disable() = 0;

      protected:
        virtual void OnEnable() = 0;
        virtual void OnDisable() = 0;
    };
} // namespace Myriad
#endif
