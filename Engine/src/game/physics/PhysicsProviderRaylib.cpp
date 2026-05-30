#include "game/physics/PhysicsProviderRaylib.h"
#if MYRIAD_RENDERER == RENDERER_RAYLIB
    #include "raylib.h"
#endif

namespace Myriad::Physics
{
    PhysicsProviderRaylib::PhysicsProviderRaylib() : IPhysicsProvider() {}
    PhysicsProviderRaylib::~PhysicsProviderRaylib() {}
    bool PhysicsProviderRaylib::Init() { return true; }
    bool PhysicsProviderRaylib::Shutdown() { return true; }

    bool PhysicsProviderRaylib::CollideRect2D(Rect2D rect1, Rect2D rect2)
    {
        Rectangle r1 = {rect1.Left(), rect1.Top(), rect1.size.x, rect1.size.y};
        Rectangle r2 = {rect2.Left(), rect2.Top(), rect2.size.x, rect2.size.y};

        return ::CheckCollisionRecs(r1, r2);
    }
} // namespace Myriad::Physics
