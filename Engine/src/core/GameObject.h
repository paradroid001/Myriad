#ifndef __GAMEOBJECT_H_
#define __GAMEOBJECT_H_

#include <list>
#include <string>

#include "Component.h"
// Don't include entity manager
// #include "Entities/EntityManager.h"
#include "GameObjectBase.h"
#include "Transform.h"
#include "core.h"

namespace Myriad
{
    class GameObject;
    struct GameObjectData : ComponentData
    {
        GameObject *gameObject;
    };

    class MYR_API GameObject : public GameObjectBase
    {
      public:
        GameObject();
        virtual ~GameObject();

        /* THE OLD WAY
        // This needs to be inlined in the header, or you get linking errors
        template <typename U>
        inline int AddComponent(Component *c) //, ComponentData *d)
        {
            entity.add<U>(); //  add this component type to the entity?
            ComponentData *pd = c->Data();
            U *castObj = static_cast<U *>(pd);
            entity.set<U>(*castObj);

            // The component now has a game object
            c->SetGameObject(this);
            components.push_front(*c);
            return 0;
        }
        */

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

        // Transform *const GetTransform();
        // std::string GetName();
        // void SetName(const char *newname);

        // virtual void Update();

        // TODO this is just for testing.
        virtual void Draw();
        // virtual void DrawCircleShape(float x, float y, float r, MyrColour c);
        // Myriad::Entities::Entity &Entity() { return entity; };
        // GameObjectData *Data() { return &_internalData; }

      protected:
        // GameObjectData _internalData;
        // Myriad::Entities::Entity entity;

        std::list<ComponentBase *> m_components;
        std::string name;
        Myriad::Transform m_transform;
    };
} // namespace Myriad

#endif