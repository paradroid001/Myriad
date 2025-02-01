#include "game/oc/Component.h"
#include "game/oc/GameObject.h"
#include "game/oc/GameObjectManager.h"
#include "game/oc/ComponentManager.h"

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
}
