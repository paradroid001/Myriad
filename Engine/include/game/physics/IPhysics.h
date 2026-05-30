#ifndef _MYRIAD_PHYSICS_IPHYSICS_H_
#define _MYRIAD_PHYSICS_IPHYSICS_H_

#include "core/core.h" //rect2d

namespace Myriad::Physics
{
    class IPhysics
    {
      public:
        virtual ~IPhysics() = 0;
        virtual bool CollideRect2D(Rect2D rect1, Rect2D rect2) = 0;
    };
} // namespace Myriad::Physics
#endif
