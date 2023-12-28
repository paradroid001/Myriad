#ifndef __COMPONENT_H_
#define __COMPONENT_H_

#include "ComponentBase.h"
#include "core.h"
#include <iostream>

namespace Myriad
{

    class Component; // fwd
    class GameObject;

    typedef struct
    {
        bool enabled;
        Component *p_component;
    } ComponentData;

    class MYR_API Component : public ComponentBase
    {
      public:
        Component();
        virtual ~Component();
        virtual void Init();
        //  GameObject *parent();
        void Enable() override;
        void Disable() override;
        // void Exec(); // whatever this component does, perhaps nothing.

        void OnEnable() override;
        void OnDisable() override;

        inline void SetGameObject(GameObject *pgo)
        {
            mp_gameObject = (GameObjectBase *)pgo;
        }; // TODO: not if already assigned?

        inline const GameObjectBase *GetGameObject() override
        {
            return (const GameObjectBase *)mp_gameObject;
        };

        // TODO: not if already assigned?
        // TODO what about making this pure virtual and forcing defininition
        // in decendents? At least the first part, then people can't forget
        // to call it.in constructor
        virtual void SetComponentData(ComponentData *data)
        {
            //_pdata = data;
            //_pdata->pcomponent = this;
        }

        // virtual ComponentData *Data() { return _pdata; };

      protected:
        // ComponentData *_pdata;
        bool _enabled;
        GameObjectBase *mp_gameObject;
    };
} // namespace Myriad
#endif