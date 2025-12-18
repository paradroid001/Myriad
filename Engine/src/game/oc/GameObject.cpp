#include "game/oc/GameObject.h"
#include "game/oc/GameObjectManager.h"
#include "game/oc/ComponentManager.h"
#include "gfx/Renderer.h"
#include "core/MyrProfiling.h"

namespace Myriad::ObjectComponent
{

  // because we have mem initialiser here, define full initialisation
  GameObject::GameObject(MYR_ID_t id) : IMyrGameObject(), id_(id)
  {
  }
  // default constructor is declared after, and delegates to the other one.
  // this is because you can't delegate AND have mem initalisers.
  // See https://stackoverflow.com/questions/12190051/member-initialization-while-using-delegated-constructor
  GameObject::GameObject() : GameObject(MYRIAD_INVALID_ID) {}
  GameObject::~GameObject()
  {
  }

  void GameObject::Update(float dt)
  {
    MyrProfileScoped(GameObject__Update);
    for (auto p_component : v_updateable_)
    {
      if (p_component != nullptr && p_component->GetUpdateable())
      {
        p_component->Update(dt);
      }
    }
  }
  void GameObject::Render(Renderer &renderer)
  {
    MyrProfileScoped(GameObject__Render);
    for (auto p_component : v_renderable_)
    {
      if (p_component != nullptr && p_component->GetRenderable())
      {
        p_component->Render(renderer);
      }
    }
  }

  MYR_ID_t GameObject::GetID() const { return id_; }
  void *GameObject::GetManager() { return nullptr; }

  GameObjectManager *GameObject::s_object_manager_ = nullptr;
  ComponentManager *GameObject::s_component_manager_ = nullptr;
}
