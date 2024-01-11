#ifndef __GAMEENTITY_H_
#define __GAMEENTITY_H_

#include "IComponentContainer.h"
#include "IEnableable.h"
#include "core.h"

#include "GameObjectBase.h"
#include "core/ComponentBase.h"
#include "entities/EntityManager.h"

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

        int GetNumChildren() override;
        GameObjectBase **GetChildren() override;
        GameObjectBase *GetChild(int i) override;
        int AddChild(GameObjectBase *child) override;
        int RemoveChild(GameObjectBase *child) override;
        int RemoveChildn(int n) override;
    };
} // namespace Myriad

#endif