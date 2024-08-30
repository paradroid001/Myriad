#ifndef MYRIAD_CORE_IDRAWABLE_H
#define MYRIAD_CORE_IDRAWABLE_H

#include "core/core.h"
#include "rendering/Renderer.h"

namespace Myriad
{
    class MYR_API IDrawable
    {
      public:
        virtual void Draw(Renderer &renderer) = 0;
    };
} // namespace Myriad
#endif
