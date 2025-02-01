#ifndef MYRIAD_GAME_GAMEOJECT_H
#define MYRIAD_GAME_GAMEOJECT_H

#include "core/core.h"

#include "game/IMyrGameObject.h"
#include "game/oc/ComponentManager.h"

#include "gfx/Renderer.h"
#include <cstdint>
#include <vector>

namespace Myriad::ObjectComponent
{
  using namespace Myriad;
  // Fwd declare
  class GameObjectManager;

  class MYR_API GameObject : public IMyrGameObject
  {
    friend class GameObjectManager; // for SetID

  protected:
    MYR_ID_t id_;
    static GameObjectManager *s_object_manager_;
    static ComponentManager *s_component_manager_;
    std::vector<ComponentBase *> v_updateable_;
    std::vector<ComponentBase *> v_renderable_;
    // Friend class GameObjectManager can call this.
    inline void SetID(MYR_ID_t id) { id_ = id; };

  public:
    GameObject();
    GameObject(MYR_ID_t id);
    virtual ~GameObject();

    void Update(float dt);
    void Render(Renderer &renderer);

    static void SetObjectManager(GameObjectManager *ptr)
    {
      if (s_object_manager_ == nullptr)
        s_object_manager_ = ptr;
      else
        MYR_CORE_ERROR("GameObject::s_object_manager was already set!");
    }
    static void SetComponenttManager(ComponentManager *ptr)
    {
      if (s_component_manager_ == nullptr)
        s_component_manager_ = ptr;
      else
        MYR_CORE_ERROR("GameObject::s_component_manager was already set!");
    }

    MYR_ID_t GetID() const override;
    void *GetManager() override; // const seems hard when returning pointers.

    // Children
    // virtual bool AddChild(MyrObjectID_t childid) override;
    // virtual bool RemoveChild(MyrObjectID_t childid) override;
    // virtual bool RemoveChildByIndex(uint32_t index) override;
    // Components
    template <typename T, typename... Args>
    bool AddComponent(Args... args)
    {
      if (s_component_manager_ != nullptr)
      {
        MYR_ID_t component_id = s_component_manager_->CreateObject<T>();
        T *p_component = s_component_manager_->Get<T>(component_id);
        if (p_component != nullptr)
        {
          p_component->SetOwner(this);
          p_component->Init(args...);
          if (p_component->GetRenderable())
            v_renderable_.push_back(p_component);
          if (p_component->GetUpdateable())
            v_updateable_.push_back(p_component);
          return true;
        }
      }
      return false;
    }

    template <typename ComponentType>
    ComponentType *GetComponent()
    {
      return s_component_manager_->GetComponent<ComponentType>(id_);
    }
    // virtual bool RemoveComponent(MyrComponentID_t component) override;
    // virtual MyrComponentID_t GetComponent(MyrComponentType_t component_type) override;
  };
}

#endif
