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
    } // namespace Events
} // namespace Myriad