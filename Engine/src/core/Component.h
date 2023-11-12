#ifndef __COMPONENT_H_
#define __COMPONENT_H_

#include "core.h"

namespace Myriad
{

    class Component; // fwd
    class GameObject;
    typedef struct
    {
        bool enabled;
        Component *pcomponent;
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

        inline void SetGameObject(GameObject *pgo)
        {
            _pgameObject = pgo;
        }; // TODO: not if already assigned?

        // TODO: not if already assigned?
        // TODO what about making this pure virtual and forcing defininition in
        // decendents? At least the first part, then people can't forget to call
        // it.in constructor
        virtual void SetComponentData(ComponentData *data)
        {
            _pdata = data;
            _pdata->pcomponent = this;
        }

        virtual ComponentData *Data() { return _pdata; };

      protected:
        ComponentData *_pdata;
        bool _enabled;
        GameObject *_pgameObject;
    };
} // namespace Myriad
#endif