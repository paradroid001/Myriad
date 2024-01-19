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
        virtual void Add(T &) = 0;
        virtual void Remove(T &) = 0;
        virtual int Count() = 0;
        virtual T *Iterate() = 0;
    };
} // namespace Myriad

#endif