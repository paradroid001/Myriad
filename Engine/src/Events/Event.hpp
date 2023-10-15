#ifndef __EVENT_HPP_
#define __EVENT_HPP_
// #include "Events/Event.h"

namespace Myriad
{
    namespace Events
    {

        template <class T> void Event::Call()
        {
            Myriad::Events::EventDispatcher::Instance().Call<T>(this);
        }

        // static
        template <class T> void Event::Register(EventCallback<T> handler)
        {
            Myriad::Events::EventDispatcher::Instance().Register(handler);
        }
        // static
        template <class T> void Event::Unregister(EventCallback<T> handler)
        {
            Myriad::Events::EventDispatcher::Instance().Unregister(handler);
        }

    } // namespace Events
} // namespace Myriad

#endif