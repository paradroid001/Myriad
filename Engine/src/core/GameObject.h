#ifndef __GAMEOBJECT_H_
#define __GAMEOBJECT_H_

#include <list>
#include <string>

#include "Component.h"
#include "Entities/EntityManager.h"
#include "Transform.h"
#include "core.h"

namespace Myriad
{
    class GameObject;
    struct GameObjectData : ComponentData
    {
        GameObject *gameObject;
    };

    class MYR_API GameObject
    {
      public:
        GameObject();
        virtual ~GameObject();
        template <typename U>

        // This needs to be inlined, or you get linking errors
        inline int AddComponent(Component *c, ComponentData *d)
        {
            entity.add<U>(); //  add this component type to the entity?
            ComponentData *pd = d;
            U *castObj = static_cast<U *>(pd);
            entity.set<U>(*castObj);

            components.push_front(*c);
            return 0;
        }

        // int AddComponent(Component *c);
        //  int AddComponent(Myriad::Component c);
        int RemoveComponent(Component c);
        // template<> Component GetComponent<Component>();
        Transform *const GetTransform();
        std::string GetName();
        void SetName(const char *newname);
        virtual void Update();
        virtual void Draw();
        virtual void DrawCircleShape(float x, float y, float r, MyrColour c);
        Myriad::Entities::Entity &Entity() { return entity; };
        GameObjectData *Data() { return &_internalData; }

      protected:
        class GameObjectData _internalData;
        Myriad::Entities::Entity entity;

        std::list<Component> components;
        std::string name;
        Myriad::Transform *_ptransform;
    };
} // namespace Myriad

#endif