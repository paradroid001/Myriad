#ifndef __COMPONENTBASE_H_
#define __COMPONENTBASE_H_

// #include "GameObjectBase.h"
#include "core.h" //for MYR_API
#include "core/IEnableable.h"
#include <string>

namespace Myriad
{
    class GameObjectBase;

    class MYR_API ComponentBase : public IEnableable
    {
      public:
        virtual ~ComponentBase() {}

        // Satisfy IEnableable
        virtual void Enable() = 0;
        virtual void Disable() = 0;
        virtual void OnEnable() = 0;
        virtual void OnDisable() = 0;

        virtual std::string &GetName() = 0;
        virtual const GameObjectBase *GetGameObject() = 0;
        virtual const void SetGameObject(GameObjectBase *gob) = 0;
    };
} // namespace Myriad

#endif