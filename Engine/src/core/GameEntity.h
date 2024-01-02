#ifndef __GAMEENTITY_H_
#define __GAMEENTITY_H_

#include "IComponentContainer.h"
#include "IEnableable.h"
#include "core.h"

#include "Entities/EntityManager.h"
#include "core/ComponentBase.h"
#include "core/GameObjectBase.h"
// TODO would be nice to not have to include flecs.
// Everything should be in EntityManager
#include "flecs.h"

namespace Myriad
{
    class MYR_API GameEntity : public GameObjectBase
    {
      public:
        GameEntity();
        virtual ~GameEntity();

        void AddComponent(ComponentBase *c) override;
        void RemoveComponent(ComponentBase *c) override;
        ComponentBase *GetComponentByName(std::string &name) override;

        void Enable() override;
        void Disable() override;
        void OnEnable() override;
        void OnDisable() override;

        int GetNumChildren();
        GameObjectBase **GetChildren();
        GameObjectBase *GetChild(int i);
        int AddChild(GameObjectBase *child);
        int RemoveChild(GameObjectBase *child);
        int RemoveChildn(int n);
    };
} // namespace Myriad

#endif