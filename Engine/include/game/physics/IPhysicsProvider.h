#ifndef _MYRIAD_PHYSICS_IPHYSICSPROVIDER_
#define _MYRIAD_PHYSICS_IPHYSICSPROVIDER_

#include "core/core.h"

#include "core/IProvider.h"
#include "game/physics/IPhysics.h"

namespace Myriad::Physics
{
    class MYR_API IPhysicsProvider : public IPhysics, public IProvider
    {
      public:
        virtual ~IPhysicsProvider() = 0;

        // IProvider methods
        virtual bool Init() override = 0;
        virtual bool Shutdown() override = 0;

        // IPhysics Methods
        virtual bool CollideRect2D(Rect2D rect1, Rect2D rect2) override = 0;
    };
} // namespace Myriad::Physics
#endif
