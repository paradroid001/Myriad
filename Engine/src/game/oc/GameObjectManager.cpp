#include "game/oc/GameObjectManager.h"

namespace Myriad::ObjectComponent
{
  GameObjectManager::GameObjectManager(MYR_ID_t max_slots) : MyrObjectManager<GameObject, GOAlloc_t>()
  {
    p_allocator_ = new GOAlloc_t(max_slots, this);
  }
  GameObjectManager::~GameObjectManager()
  {
    delete p_allocator_;
  }

  void GameObjectManager::DestroyObjectById(MYR_ID_t id)
  {
    p_allocator_->Destroy(id);
  }
  GameObject *GameObjectManager::GetObject(MYR_ID_t id)
  {
    return p_allocator_->Get(id);
  }
  void GameObjectManager::OnObjectCreate(MYR_ID_t id, void *ptr)
  {
    MYR_CORE_TRACE("Game Object manager notified of creation, id: {0}", id);
    // If this line isn't here, no gameobjects get their ids set on creation.
    static_cast<GameObject *>(ptr)->SetID(id);
  };
}
