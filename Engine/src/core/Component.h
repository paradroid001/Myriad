#ifndef __COMPONENT_H_
#define __COMPONENT_H_

#include "core.h"

namespace Myriad
{

    class Component; // fwd
    typedef struct
    {
        bool enabled;
        Component *component;
    } ComponentData;

    class MYR_API Component
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

        virtual ComponentData *Data() { return &_data; };

      private:
        bool _enabled;
        // GameObject *_gameObject;
      protected:
        ComponentData _data;
    };
} // namespace Myriad
#endif