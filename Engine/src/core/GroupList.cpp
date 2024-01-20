#include "core/GroupList.h"

namespace Myriad
{
    template <class T> GroupList<T>::GroupList()
    {
        m_iterating = false;
    }
    template <class T> GroupList<T>::~GroupList()
    {

    }
    template <class T> void GroupList<T>::Add(T& item)
    {
        m_itemList.push_back(&item);
    }
    template <class T> void GroupList<T>::Remove(T& item)
    {
        m_itemList.remove(&item);
    }
    template <class T> int GroupList<T>::Count() { return m_itemList.size();}

    template <class T> T* GroupList<T>::Iterate()
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

}