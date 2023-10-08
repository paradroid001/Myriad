#include "core/core.h"

namespace Myriad
{
    namespace Event
    {
        //"Scoped" enum: stronger type enforcing, no leaked names.
        enum class EventType
        {
            None = 0,

            WindowResize,
            WindowMove,
            WindowClose,

            KeyPress,
            KeyRelease,

            MouseButtonPress,
            MouseButtonRelease,
            MouseMove,
            MouseScroll,

            EngineRender,
            EngineTick,
            EngineUpdate

            // File system?
            // Network?
            // Audio?
        } enum EventCategory
        {
            None = 0,
            Engine = BIT(0),
            Input = BIT(1),
            Keyboard = BIT(2),
            Mouse = BIT(3),
        }

        class Event
        {
            virtual EventType GetEventType() const = 0;
            virtual const char *GetName() const = 0;
            virtual int GetCategoryFlags() const = 0;
            inline bool IsInCategory(EventCategory category)
            {
                return GetCategoryFlags() & category;
            }
        }
    } // namespace Event
} // namespace Myriad