#ifndef _PHYSICS_PROVIDER_RAYLIB_H_
#define _PHYSICS_PROVIDER_RAYLIB_H_

#include "game/physics/IPhysicsProvider.h"

namespace Myriad::Physics
{

    class PhysicsProviderRaylib : public IPhysicsProvider
    {
      public:
        PhysicsProviderRaylib();
        ~PhysicsProviderRaylib();
        bool Init() override;
        bool Shutdown() override;
        bool CollideRect2D(Rect2D rect1, Rect2D rect2) override;
    };

} // namespace Myriad::Physics

#endif
