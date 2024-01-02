#ifndef __COMPONENTLIST_H_
#define __COMPONENTLIST_H_

#include "ComponentBase.h"
#include "IGroup.h"
#include "core.h"

namespace Myriad
{
    class MYR_API ComponentList : public IGroup<ComponentBase>
    {
      public:
        ComponentList();
        ~ComponentList();
        virtual void add<ComponentBase & component>() override;
        virtual remove<ComponentBase & component>() override;
        virtual count<ComponentBase & component>() override;
        virtual ComponentBase *iterate() override;
    };
} // namespace Myriad

#endif