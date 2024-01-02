#ifndef __GAMEOBJECTBASE_H_
#define __GAMEOBJECTBASE_H_

#include "ComponentBase.h"
#include "IComponentContainer.h"
#include "IEnableable.h"
#include "core.h"
#include <iostream>

namespace Myriad
{
    class MYR_API GameObjectBase : public IEnableable,
                                   public IComponentContainer
    {
      public:
        // TODO: does this class need a virtual destructor
        // (plus implementation)?
        // virtual ~GameObjectBase();

        virtual int GetNumChildren() = 0;
        // TODO I feel like this should actually be an
        // array of references, but see below..
        virtual GameObjectBase **GetChildren() = 0;
        // TODO I don't like returning a pointer, but I
        // can't use a reference because the types won't match..
        // GameObjectBase is an abstract class.
        virtual GameObjectBase *GetChild(int i) = 0;
        virtual int AddChild(GameObjectBase *child) = 0;
        virtual int RemoveChild(GameObjectBase *child) = 0;
        virtual int RemoveChildn(int n) = 0;

        // For IComponentContainer
        // virtual void AddComponent(ComponentBase *c) = 0;
        // virtual void RemoveComponent(ComponentBase *c) = 0;
        // virtual ComponentBase* GetComponentByName(std::string& name) = 0;
        // template <typename T> const ComponentBase *GetComponent();

        // For IEnableable
        // virtual void Enable() = 0;
        // virtual void Disable() = 0;
        // virtual void OnEnable() = 0;
        // virtual void OnDisable() = 0;
    };
} // namespace Myriad

#endif
