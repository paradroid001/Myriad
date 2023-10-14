#include "Events/Event.h"
#include "core/Log.h"
#include <vector>

namespace Myriad
{
    namespace Events
    {

        template <class T> void EventContainer<T>::Call(T e)
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
            events(e); // call the multicast delegate
        }
        template <class T> void EventContainer<T>::Add(EventCallback<T> d)
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
            events += d.Callback();
        }
        template <class T> void EventContainer<T>::Remove(EventCallback<T> d)
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
            events -= d.Callback();
        }
    } // namespace Events
} // namespace Myriad