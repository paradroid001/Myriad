#ifndef __IUPDATEABLE_H_
#define __IUPDATEABLE_H_

#include "core/core.h"

namespace Myriad
{
    class MYR_API IUpdateable
    {
        public:
            virtual void Update(float dt) = 0;
    };
}

#endif