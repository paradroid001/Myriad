#include "game/oc/Component.h"
#include "game/oc/ComponentManager.h"
#include "game/oc/GameObject.h"
#include "game/oc/GameObjectManager.h"

namespace Myriad::ObjectComponent
{
    MYR_ID_t ComponentBase::GetOwnerId()
    {
        if (owner_ != nullptr)
        {
            return owner_->GetID();
        }
        return MYRIAD_INVALID_ID;
    }

    GameObjectManager *ComponentBase::s_object_manager_ = nullptr;
    ComponentManager *ComponentBase::s_component_manager_ = nullptr;
    AssetManager *ComponentBase::s_asset_manager_ = nullptr;

    uint16_t ComponentBase::nextComponentType = 0;
    uint16_t ComponentBase::GetNextComponentType()
    {
        return ++nextComponentType;
    }

} // namespace Myriad::ObjectComponent
