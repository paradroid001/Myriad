#ifndef MYRIAD_GAME_GAMEOBJECTCOMPONENT_H
#define MYRIAD_GAME_GAMEOBJECTCOMPONENT_H

#include "core/core.h"
#include "gfx/Renderer.h"
#include "asset/AssetManager.h"

namespace Myriad::ObjectComponent
{
  // fwd
  class GameObject;
  class GameObjectManager;
  class ComponentManager;

  class MYR_API ComponentBase
  {
    friend class ComponentManager; // so manager can call SetID
    friend class GameObject;       // so gameobjects can call SetOwner

  protected:
    MYR_ID_t id_;
    GameObject *owner_;
    bool is_updateable_; // default true
    bool is_renderable_; // default false

    // These static vars are so that components can access their
    // own manager and the asset manager easily.
    static GameObjectManager *s_object_manager_;
    static ComponentManager *s_component_manager_;
    static AssetManager *s_asset_manager_;

    inline void SetID(MYR_ID_t id) { id_ = id; }
    inline void SetOwner(GameObject *owner) { owner_ = owner; }

  public:
    static void SetObjectManager(GameObjectManager *ptr)
    {
      if (s_object_manager_ == nullptr)
        s_object_manager_ = ptr;
      else
        MYR_CORE_ERROR("Component::s_object_manager was already set!");
    }
    static void SetComponentManager(ComponentManager *ptr)
    {
      if (s_component_manager_ == nullptr)
        s_component_manager_ = ptr;
      else
        MYR_CORE_ERROR("Component::s_component_manager was already set!");
    }

    inline static void SetAssetManager(AssetManager *ptr)
    {
      if (s_asset_manager_ == nullptr)
        s_asset_manager_ = ptr;
      else
        MYR_CORE_ERROR("Component::s_asset_manager was already set!");
    }

    ComponentBase(MYR_ID_t id, GameObject *owner, bool updateable, bool renderable)
        : id_(id), owner_(owner), is_updateable_(updateable), is_renderable_(renderable) {}

    // Default
    ComponentBase() : ComponentBase(MYRIAD_COMPONENT_INVALID_ID, nullptr, true, false) {};
    virtual ~ComponentBase() {};

    void Init() {}; // this gets overloaded in every class

    inline MYR_ID_t GetID() const { return id_; }
    inline GameObject *GetOwner() { return owner_; }
    MYR_ID_t GetOwnerId();

    virtual void OnEnable() {};
    virtual void OnDisable() {};

    virtual void Update(float dt) {};
    virtual void Render(Myriad::Renderer &renderer) {};

    inline void SetUpdateable(bool v) { is_updateable_ = v; }
    inline bool GetUpdateable() { return is_updateable_; }
    inline void SetRenderable(bool v) { is_renderable_ = v; }
    inline bool GetRenderable() { return is_renderable_; }
  };

  template <typename T>
  class Component : public ComponentBase
  {
  protected:
    static uint16_t type;

  public:
    static uint16_t Type() { return T::type; }
    uint16_t GetType() const { return T::type; }
  };

  static uint16_t nextComponentType = 0;
  template <typename T>
  uint16_t Component<T>::type(nextComponentType++);
}

#endif
