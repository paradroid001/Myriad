#ifndef __GAMEOBJECT_H_
#define __GAMEOBJECT_H_

// Include the config
#include "MyriadConfig.h"
// The rest

// #include "../../vendor/raylib/src/raylib.h" //for vector2 etc
#include "Component.h"
// class Component;

#include "Transform.h"
#include "core.h"
#include <list>
#include <string>

namespace Myriad
{
    class MYR_API GameObject
    {
      public:
        GameObject();
        virtual ~GameObject();
        int AddComponent(Myriad::Component c);
        int RemoveComponent(Myriad::Component c);
        // template<> Component GetComponent<Component>();
        Transform *const GetTransform();
        std::string GetName();
        void SetName(const char *newname);
        virtual void Update();
        virtual void Draw();

      protected:
        std::list<Component> components;
        std::string name;
        Myriad::Transform *_ptransform;
    };
} // namespace Myriad

#endif