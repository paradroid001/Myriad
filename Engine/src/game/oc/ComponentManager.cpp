#include "game/oc/ComponentManager.h"

namespace Myriad::ObjectComponent
{
    ComponentManager::ComponentManager(MYR_ID_t max_slots)
    {
        p_allocator_ = new TypeAllocatorDynamic<ComponentBase>(max_slots, this);
    }

    ComponentManager::~ComponentManager()
    {
        MYR_CORE_TRACE("Component Manager Destructor");
        delete p_allocator_;
    }

    void ComponentManager::DestroyObjectById(MYR_ID_t id)
    {
        p_allocator_->Destroy(id);
    }
    ComponentBase *ComponentManager::GetObject(MYR_ID_t id)
    {
        return p_allocator_->Get(id);
    }

    void ComponentManager::OnObjectCreate(MYR_ID_t id, void *ptr)
    {
        MYR_CORE_TRACE("Component manage notified of creation, id: {0}", id);
        // If we don't do this, no components have ids!
        static_cast<ComponentBase *>(ptr)->SetID(id);
    }
} // namespace Myriad::ObjectComponent
