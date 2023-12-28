#ifndef __COMPONENTBASE_H_
#define __COMPONENTBASE_H_

#include "core.h" //for MYR_API
// #include "GameObjectBase.h"

namespace Myriad
{
    class GameObjectBase;

    class MYR_API ComponentBase
    {
      public:
        virtual ~ComponentBase() {}
        virtual void Enable() = 0;
        virtual void Disable() = 0;
        virtual void OnEnable() = 0;
        virtual void OnDisable() = 0;
        virtual const GameObjectBase *GetGameObject();
    };
} // namespace Myriad

#endif