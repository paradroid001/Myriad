#ifndef __IGROUP_H_
#define __IGROUP_H_

#include "core.h"

namespace Myriad
{
    template <class T> class MYR_API IGroup
    {
      public:
        // TODO do I really need this destructor?
        virtual ~IGroup(){};
        virtual void add(T &) = 0;
        virtual void remove(T &) = 0;
        virtual int count() = 0;
        virtual T *iterate() = 0;
    };
} // namespace Myriad

#endif