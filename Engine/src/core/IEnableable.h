#ifndef __IENABLEABLE_H_
#define __IENABLEABLE_H_

#include "core.h"

namespace Myriad
{
    class MYR_API IEnableable
    {
        public:
            virtual void Enable() = 0;
            virtual void Disable() = 0;
            virtual void OnEnable() = 0;
            virtual void OnDisable() = 0;
    };
}

#endif