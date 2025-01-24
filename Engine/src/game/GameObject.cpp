#include "game/GameObject.h"

namespace Myriad
{
  GameObject::GameObject(MyrObjectID_t id) : MyrGameObject(), id_(id)
  {
  }
  GameObject::~GameObject()
  {
  }

  MyrObjectID_t GameObject::GetID() const { return id_; }
  MyrObjectManager *GameObject::GetManager() { return manager_; }

}
