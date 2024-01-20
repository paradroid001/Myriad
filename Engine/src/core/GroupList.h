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
            GroupList()
            {
                m_iterating = false;
            }
            virtual ~GroupList()
            {

            }
            virtual void Add(T& item)
            {
                m_itemList.push_back(&item);
            }
            virtual void Remove(T& item)
            {
                m_itemList.remove(&item);
            }
            virtual int Count() { return m_itemList.size();}

            virtual T* Iterate()
            {
                if (!m_iterating)
                {
                    m_iterator = m_itemList.begin();
                    m_iterating = true;
                }
                else
                {
                    ++m_iterator;
                }

                if (m_iterator == m_itemList.end())
                {
                    m_iterating = false;
                    return NULL;
                }
                return (T *)*m_iterator;
            }
        protected:
            std::list<T *> m_itemList;
            std::list<T *>::iterator m_iterator;
            bool m_iterating;
    };
}


#endif