#ifndef MYRIAD_CORE_GAMEOJECT_H
#define MYRIAD_CORE_GAMEOJECT_H

#include "core/core.h"
#include "core/MyrGameObject.h"
#include "game/GameObjectManager.h"
#include <cstdint>

namespace Myriad
{
  class MYR_API GameObject : public MyrGameObject
  {
  protected:
    MyrObjectID_t id_;
    GameObjectManager<GameObject> *manager_;

  public:
    GameObject(MyrObjectID_t id);
    virtual ~GameObject();

    friend GameObjectManager<GameObject>;

    MyrObjectID_t GetID() const override;
    MyrObjectManager *GetManager() override; // const seems hard when returning pointers.

    // Children
    // virtual bool AddChild(MyrObjectID_t childid) override;
    // virtual bool RemoveChild(MyrObjectID_t childid) override;
    // virtual bool RemoveChildByIndex(uint32_t index) override;
    // Components
    // virtual bool AddComponent(MyrComponentID_t componentid) override;
    // virtual bool RemoveComponent(MyrComponentID_t component) override;
    // virtual MyrComponentID_t GetComponent(MyrComponentType_t component_type) override;
  };
}

#endif
