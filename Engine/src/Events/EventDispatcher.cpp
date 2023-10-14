#include "Events/Event.h"
#include <map>
#include <string>

namespace Myriad
{
    namespace Events
    {

        // private constructor
        EventDispatcher::EventDispatcher()
        {
            _pRegistrants = new std::map<std::string, IEventContainer *>();
        }

        EventDispatcher &EventDispatcher::Instance()
        {
            if (_instance == NULL)
            {
                _instance = new EventDispatcher();
            }
            return (EventDispatcher &)*_instance;
        }

        template <class T>
        void EventDispatcher::Register(EventCallback<T> handler)
        {
            const char *name = typeid(T).name();
            if (_pRegistrants->find(name) == _pRegistrants->end())
            {
                (*_pRegistrants)[name] = new EventContainer<T>();
            }
            (*_pRegistrants)[name]->Add(handler);
        }
        template <class T>
        void EventDispatcher::Unregister(EventCallback<T> handler)
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