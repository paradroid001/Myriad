#ifndef __COMPONENT_H_
#define __COMPONENT_H_

#include "core.h"

namespace Myriad
{
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

      private:
        bool _enabled;
        // GameObject *_gameObject;
    };
} // namespace Myriad
#endif