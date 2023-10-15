#ifndef __EVENTCONTAINER_HPP_
#define __EVENTCONTAINER_HPP_

// #include "Events/Event.h"
#include "core/Log.h"
#include <iostream>
#include <vector>

namespace Myriad
{
    namespace Events
    {

        template <class T> void EventContainer<T>::Call(IEvent e)
        {
            /*
            if (events.size() != 0)
            {
                std::vector<std::function<void(T)>>::iterator it;
                for (it = events.begin(); it != events.end(); ++it)
                {
                    (*(*it))(e); // call the callback.
                }
            }
            */
            // events(e); // call the multicast delegate
            std::cout << "Here I have to somehow call the delegate."
                      << std::endl;
        }
        template <class T> void EventContainer<T>::Add(IEventCallback d)
        {
            /*
            CallbackArray::iterator position =
                find(events.begin(), events.end(), d);
            if (position != events.end())
            {
                MYR_WARN("EventContainer::Add - Event existed in delegate list "
                         "already!. Not adding.");
            }
            else
            {
                events.push_back(d);
            }
            */
            // events += d.callback; // d.Callback();
            EventCallback<T> ecb = static_cast<EventCallback<T>>(d);
            events += ecb.callback;
        }
        template <class T> void EventContainer<T>::Remove(IEventCallback d)
        {
            /*
            CallbackArray::iterator position =
                find(events.begin(), events.end(), d);
            if (position == events.end())
            {
                MYR_WARN("EventContainer::Remove - Event did not exist in "
                         "delegate list.");
            }
            else
            {
                events.erase(position);
            }
            */
            // events -= d.callback; // d.Callback();
            EventCallback<T> ecb = static_cast<EventCallback<T>>(d);
            events -= ecb.callback;
        }
    } // namespace Events
} // namespace Myriad
#endif