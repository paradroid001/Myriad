#include "core/core.h"

#include "game/oc/GameObjectManager.h"
#include "game/oc/Physics2D.h"
#include "game/physics/Physics.h"

#if MYRIAD_RENDERER == RENDERER_RAYLIB
    #include "game/physics/PhysicsProviderRaylib.h"
#endif

namespace Myriad::Physics
{
    using namespace Myriad::ObjectComponent;

    // Abstract virtual destructors should be defined outside the class
    // This is a convenient place to do it, rather than creating more cpp files.
    IPhysicsProvider::~IPhysicsProvider() {}
    IPhysics::~IPhysics() {}

    PhysicsSystem::PhysicsSystem() : IPhysics()
    {
        MYR_CORE_TRACE("PhysicsSystem constructor");
#if MYRIAD_RENDERER == RENDERER_RAYLIB
        p_physics_provider_ = new PhysicsProviderRaylib();
#endif
        for (int i = 0; i < PHYSICS_LAYERS_MAX; i++)
        {
            layerMasks[i] = 0;
        }
    }

    PhysicsSystem::~PhysicsSystem() { delete p_physics_provider_; }

    void PhysicsSystem::SetLayerMask(PhysicsLayer_t layer,
                                     PhysicsLayerMask_t mask)
    {
        // only layers 0-(max-1) are valid
        if (layer < PHYSICS_LAYERS_MAX)
        {
            layerMasks[layer] = mask;
            MYR_CORE_TRACE("Set layermask for {0} to {1}", layer, mask);
        }
    }

    PhysicsLayerMask_t PhysicsSystem::GetLayerMask(PhysicsLayer_t layer)
    {
        PhysicsLayerMask_t retval = 0;
        // only layers 0-(max-1) are valid
        if (layer < PHYSICS_LAYERS_MAX)
        {
            retval = layerMasks[layer];
        }
        return retval;
    }

    PhysicsLayerMask_t PhysicsSystem::GetLayerMaskValue(PhysicsLayer_t layer)
    {
        PhysicsLayerMask_t retval = 0;
        if (layer < PHYSICS_LAYERS_MAX)
            retval = maskValues[layer];
        return retval;
    }

    void PhysicsSystem::CollideLayers2D(PhysicsLayerMask_t layer1,
                                        PhysicsLayerMask_t layer2,
                                        GameObjectManager *gm)
    {
        // First, do the layers even collide?
        if (layer1 < PHYSICS_LAYERS_MAX && layer2 < PHYSICS_LAYERS_MAX)
        {
            // If layer1masks is checked at layer 1 OR
            //    layer2masks is checked at layer 2
            if (layerMasks[layer1] & maskValues[layer2] ||
                layerMasks[layer2] & maskValues[layer1])
            {
                // MYR_CORE_INFO("The layers collide");
                for (auto go1 : *gm)
                {
                    for (auto go2 : *gm)
                    {
                        // if they aren't the same go.
                        if (go1 != go2)
                        {
                            Physics2D *p1 = go1->GetComponent<Physics2D>();
                            Physics2D *p2 = go2->GetComponent<Physics2D>();
                            // If they both have physics components
                            if (p1 != nullptr && p2 != nullptr)
                            {
                                Rect2D r1 = *(p1->GetRect2D());
                                Rect2D r2 = *(p2->GetRect2D());
                                // MYR_CORE_TRACE("r1: {0},{1},{2},{3}",
                                // r1.pos.x,
                                //                r1.pos.y, r1.size.x,
                                //                r1.size.y);
                                if (p_physics_provider_->CollideRect2D(r1, r2))
                                {
                                    MYR_CORE_TRACE("Collided on layer "
                                                   "{0}/{1}: "
                                                   "{2} <-> {3}",
                                                   layer1, layer2, go1->GetID(),
                                                   go2->GetID());
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    bool PhysicsSystem::CollideRect2D(Rect2D rect1, Rect2D rect2)
    {
        return p_physics_provider_->CollideRect2D(rect1, rect2);
    }
} // namespace Myriad::Physics
