#ifndef __TYPES2D_H_
#define __TYPES2D_H_

#include "MyriadConfig.h"
#include "core.h"

namespace Myriad
{
    class MYR_API Vector2 // : public ::Vector2 // raylib vector2?
    {
      public:
        // This is a mirror of the raylib type.
        float x;
        float y;
    };
} // namespace Myriad
#endif