#ifndef __GROUPLIST_H_
#define __GROUPLIST_H_

#include "core/core.h"
#include "core/IGroup.h"

#include <list>
using std::list;

namespace Myriad
{
    template <class T> class MYR_API GroupList : public IGroup<T>
    {
        public:
            GroupList();
            virtual ~GroupList();
            virtual void Add(T&);
            virtual void Remove(T&);
            virtual int Count();
            virtual T* Iterate();
        protected:
            std::list<T *> m_itemList;
            std::list<T *>::iterator m_iterator;
            bool m_iterating;
    };
}


#endif