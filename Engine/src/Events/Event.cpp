#include "Events/Event.h"

namespace Myriad
{
    namespace Events
    {
        template <class T> void Event<T>::Call()
        {
            Myriad::Events::EventDispatcher::Instance().Call<T>(this);
        }
        /*static*/ template <class T>
        void Event<T>::Register(EventCallback<T> handler)
        {
            Myriad::Events::EventDispatcher::Instance().Register(handler);
        }
        /*static*/ template <class T>
        void Event<T>::Unregister(EventCallback<T> handler)
        {
            Myriad::Events::EventDispatcher::Instance().Unregister(handler);
        }
    } // namespace Events
} // namespace Myriad