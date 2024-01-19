#ifndef __IDRAWABLE_H_
#define __IDRAWABLE_H_

#include "core/core.h"

namespace Myriad
{
    class MYR_API IDrawable
    {
        public:
            virtual void Draw() = 0;
    };
}

#endif