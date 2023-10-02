#ifndef __COMPONENT_H_
#define __COMPONENT_H_
// #include "GameObject.h"
#include "core.h"

// class GameObject;

namespace myriad
{
    class Component
    {
      public:
        Component();
        ~Component();
        virtual void Init();
        // GameObject *parent();
        void Enable();
        void Disable();
        void Exec(); // whatever this component does, perhaps nothing.

        void OnEnable();
        void OnDisable();

      private:
        bool _enabled;
        // GameObject *_gameObject;
    };
} // namespace myriad
#endif