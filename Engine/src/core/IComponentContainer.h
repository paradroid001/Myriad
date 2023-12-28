#ifndef __ICOMPONENTCONTAINER_H_
#define __ICOMPONENTCONTAINER_H_

#include "core.h"
#include "ComponentBase.h"
#include <string>

namespace Myriad
{
    class Component;

    class MYR_API IComponentContainer
    {
        public:
            virtual void AddComponent(ComponentBase *component) = 0;
            virtual void RemoveComponent(ComponentBase *component) = 0;
            //TODO this might need to be const Component * const
            virtual ComponentBase* GetComponentByName(std::string& name) = 0;
    };
}

#endif