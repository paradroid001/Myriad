#ifndef __GROUPLIST_H_
#define __GROUPLIST_H_

#include "core/core.h"
#include "core/IGroup.h"

#include <list>
using std::list;

namespace Myriad
{
    /* A group, implemented as a std::list.*/
    template <class T> class MYR_API GroupList : public IGroup<T>
    {
        public:
            GroupList()
            {
                iterating = false;
            }
            virtual ~GroupList()
            {

            }
            virtual void Add(T& item)
            {
                itemList.push_back(&item);
            }
            virtual void Remove(T& item)
            {
                itemList.remove(&item);
            }
            virtual int Count() { return itemList.size();}

            virtual T* Iterate()
            {
                if (!iterating)
                {
                    iterator = itemList.begin();
                    iterating = true;
                }
                else
                {
                    ++iterator;
                }

                if (iterator == itemList.end())
                {
                    iterating = false;
                    return NULL;
                }
                return (T *)*iterator;
            }
        protected:
            std::list<T *> itemList;
            //Requires typedef typename because T is a dependent name.
            typedef typename std::list<T *>::iterator iterator_t;
            iterator_t iterator;
            bool iterating;
    };
}


#endif