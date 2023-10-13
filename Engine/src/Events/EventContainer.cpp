#include "Events/Event.h"
#include "core/Log.h"
#include <vector>

namespace Myriad
{
    namespace Events
    {
        template <class T> void EventContainer<T>::Call(IEvent e)
        {
            if (events.size() != 0)
            {
                CallbackArray::iterator it;
                for (it = events.begin(); it != events.end(); ++it)
                {
                    (*(*it))(e); // call the callback.
                }
            }
        }
        template <class T> void EventContainer<T>::Add(EventCallback<T> d)
        {
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
        }
        template <class T> void EventContainer<T>::Remove(EventCallback<T> d)
        {
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
        }
    } // namespace Events
} // namespace Myriad