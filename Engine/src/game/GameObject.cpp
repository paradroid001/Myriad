#include "game/GameObject.h"

namespace Myriad
{
  GameObject::GameObject(MYR_ID_t id) : IMyrGameObject(), id_(id)
  {
  }
  GameObject::~GameObject()
  {
  }

  MYR_ID_t GameObject::GetID() const { return id_; }
  void *GameObject::GetManager() { return nullptr; }

}
