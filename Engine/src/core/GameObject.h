#ifndef __GAMEOBJECT_H_
#define __GAMEOBJECT_H_
#include "../../vendor/raylib/src/raylib.h" //for vector2 etc
#include "Component.h"
// class Component;

#include "Transform.h"
#include "core.h"
#include <list>
#include <string>

namespace myriad
{
    class MYR_API GameObject
    {
      public:
        GameObject();
        ~GameObject();
        int AddComponent(Component c);
        int RemoveComponent(Component c);
        // template<> Component GetComponent<Component>();
        Transform GetTransform();
        std::string GetName();
        void SetName(const char *newname);
        virtual void Update();
        virtual void Draw();

      protected:
        std::list<Component> components;
        std::string name;
        Transform *_ptransform;
    };
} // namespace myriad

#endif