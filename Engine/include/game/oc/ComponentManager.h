#ifndef MYRIAD_GAME_GAMEOBJECT_COMPONENTMANAGER_H
#define MYRIAD_GAME_GAMEOBJECT_COMPONENTMANAGER_H

#include "core/MyrAlloc.h"
#include "core/MyrObjectManager.h"
#include "core/core.h"

#include "game/oc/Component.h"

#include "io/MyrLogging.h"

#include <unordered_map>
#include <vector>

namespace Myriad::ObjectComponent
{
    using namespace Myriad;

    // Class component manager
    // This implementation holds components by using
    // a single allocator (type allocator dynamic)
    class ComponentManager
        : public MyrObjectManager<ComponentBase,
                                  TypeAllocatorDynamic<ComponentBase>>
    {
      public:
        ComponentManager(MYR_ID_t max_slots);
        ~ComponentManager();
        // Manager Methods
        void DestroyObjectById(MYR_ID_t id) override;
        ComponentBase *GetObject(MYR_ID_t id) override;
        void OnObjectCreate(MYR_ID_t id, void *ptr) override;

        // Iterator methods - iterates all components!
        auto begin() noexcept { return p_allocator_->begin(); }
        auto end() noexcept { return p_allocator_->end(); }

        // Custom methods

        template <typename ComponentType>
        ComponentType *GetComponent(MYR_ID_t owner_id)
        {
            for (auto component : *this)
            {
                if (component->GetOwnerId() == owner_id)
                {
                    ComponentType *p_typed_component =
                        static_cast<ComponentType *>(component);
                    if (p_typed_component->GetType() ==
                        ComponentType::Type()) // this needs to have resolved to
                                               // a Component class
                    {
                        return p_typed_component;
                    }
                }
            }
            return nullptr;
        }
    };
} // namespace Myriad::ObjectComponent

#endif
