#ifndef _MYRIAD_PHYSICS_PHYSICS_H_
#define _MYRIAD_PHYSICS_PHYSICS_H_

#include "core/core.h"

#include "IPhysicsProvider.h"
#include "game/oc/GameObjectManager.h"

namespace Myriad::Physics
{
    using namespace Myriad::ObjectComponent;

    // The layer indexes
    typedef enum PhysicsLayers
    {
        NONE = 0,
        UI = 1,
        DEFAULT = 2,
        CUSTOM_LAYER_1 = 3,
        CUSTOM_LAYER_2 = 4,
        CUSTOM_LAYER_3 = 5,
        CUSTOM_LAYER_4 = 6,
        CUSTOM_LAYER_5 = 7
    } PhysicsLayers_t;

    // The layer index masks
    typedef enum PhysicsLayerMasks
    {
        PHYSICS_LAYER_NONE = 0,
        PHYSICS_LAYER_UI = 1,
        PHYSICS_LAYER_DEFAULT = 2,
        PHYSICS_LAYER_CUSTOM_1 = 3,
        PHYSICS_LAYER_CUSTOM_2 = 4,
        PHYSICS_LAYER_CUSTOM_3 = 5,
        PHYSICS_LAYER_CUSTOM_4 = 6,
        PHYSICS_LAYER_CUSTOM_5 = 7
    } PhysicsLayerMasks_t;

#define PHYSICS_LAYERS_MAX 8

    typedef uint8_t PhysicsLayer_t;
    typedef uint8_t PhysicsLayerMask_t;
    class MYR_API PhysicsSystem : public IPhysics
    {

      protected:
        IPhysicsProvider *p_physics_provider_;
        // 8 masks, one for each layer.
        PhysicsLayerMask_t layerMasks[PHYSICS_LAYERS_MAX];
        // The mask values for each layer
        const PhysicsLayerMask_t maskValues[PHYSICS_LAYERS_MAX] = {
            0, 1, 2, 4, 8, 16, 32, 64};

      public:
        PhysicsSystem();
        virtual ~PhysicsSystem();
        virtual void SetLayerMask(PhysicsLayer_t layer,
                                  PhysicsLayerMask_t mask);
        // Get the current layermask for any given layer.
        virtual PhysicsLayerMask_t GetLayerMask(PhysicsLayer_t layer);
        // Get the value used as a bitmask to use in operations
        virtual PhysicsLayerMask_t GetLayerMaskValue(PhysicsLayer_t layer);
        virtual void CollideLayers2D(PhysicsLayerMask_t layer1,
                                     PhysicsLayerMask_t layer2,
                                     GameObjectManager *gm);
        virtual bool CollideRect2D(Rect2D rect1, Rect2D rect2) override;
    };
} // namespace Myriad::Physics

#endif
