#ifndef __EVENTDISPATCHER_HPP_
#define __EVENTDISPATCHER_HPP_
// #include "Events/Event.h"
#include "Events/Delegate.h"
#include <map>
#include <string>

namespace Myriad
{
    namespace Events
    {
        template <class T>
        void EventDispatcher::Register(SA::delegate<void(T)> handler)
        {
            const char *name = typeid(T).name();
            if (_pRegistrants->find(name) == _pRegistrants->end())
            {
                (*_pRegistrants)[name] = new SA::multicast_delegate<void(
                    T)>(); // new EventContainer<T>();
            }
            (*(*_pRegistrants)[name]) += handler;
        }
        template <class T>
        void EventDispatcher::Unregister(SA::delegate<void(T)> handler)
        {
        }

        template <class T> void EventDispatcher::Call(IEvent e)
        {
            const char *str_typename = typeid(T).name();
            if (_pRegistrants->find(str_typename) != _pRegistrants->end())
            {
                (*_pRegistrants)[str_typename]->Call(e);
            }
        }

    } // namespace Events
} // namespace Myriad

#endif