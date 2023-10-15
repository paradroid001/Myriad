#ifndef __EVENT_H_
    #define __EVENT_H_

    #include "Events/Delegate.h"
    #include "Events/MulticastDelegate.h"
    #include "core/core.h"
    #include <functional>
    #include <map>
    #include <string>
    #include <vector>

namespace Myriad
{
    namespace Events
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
        };

        enum EventCategory
        {
            None = 0,
            Engine = BIT(0),
            Input = BIT(1),
            Keyboard = BIT(2),
            Mouse = BIT(3),
        };

        class MYR_API IEvent
        {
          public:
            IEvent(){};
        };

        class MYR_API EventDescriptor
        {
          public:
            void *event;
        };

        class MYR_API IEventCallback
        {
          public:
            // virtual SA::delegate<void(IEvent)> Callback() = 0;
            //  virtual void operator()(IEvent) = 0;
            //  virtual bool operator==(IEventCallback *other) = 0;
        };

        template <class T> class MYR_API EventCallback : public IEventCallback
        {
          public:
            EventCallback(SA::delegate<void(T)> callback) : callback(callback)
            {
            }
            SA::delegate<void(T)> callback;
            // implement the virtual member
            // SA::delegate<void(IEvent)> Callback() { return callback; }
            /*
            virtual void operator()(IEvent e) override
            {
                (instance->*function)(e);
            }

            virtual bool operator==(IEventCallback *other) override
            {
                EventCallback *otherEventCallback =
                    dynamic_cast<EventCallback>(other);
                if (otherEventCallback == nullptr)
                    return false;

                return (this->function == otherEventCallback->function) &&
                       (this->instance == otherEventCallback->instance);
            }
            */
        };

        // This is essentially the equivalent of a delegate.
        // typedef std::vector<IEventCallback *> CallbackArray;

        class MYR_API IEventContainer
        {
          public:
            virtual void Call(IEvent e) = 0;
            virtual void Add(IEventCallback d) = 0;
            virtual void Remove(IEventCallback d) = 0;
        };

        template <class T> class MYR_API EventContainer : public IEventCallback
        {
          public:
            EventContainer(){};
            ~EventContainer(){};
            void Call(IEvent e);
            void Add(IEventCallback d);
            void Remove(IEventCallback d);

          private:
            // CallbackArray events;
            // std::function<void(T)> callback
            // typedef std::vector<std::function<void(T)>> CallbackArray;
            // std::vector<std::function<void(T)>> events;
            SA::multicast_delegate<void(T)> events;
        };

        class MYR_API EventDispatcher
        {
          private:
            EventDispatcher();
            inline static EventDispatcher *_instance = NULL;
            std::map<std::string, SA::multicast_delegate *> *_pRegistrants;

          public:
            ~EventDispatcher();
            static EventDispatcher &Instance();
            template <class T> void Register(SA::delegate<void(T)> handler);
            template <class T> void Unregister(SA::delegate<void(T)> handler);
            template <class T> void Call(IEvent e);
        };

        class MYR_API Event : public IEvent
        {
          public:
            Event() : IEvent(){};
            template <class T> void Call();
            template <class T> static void Register(EventCallback<T> handler);
            template <class T> static void Unregister(EventCallback<T> handler);
        };

        /*class Event
        {
            virtual EventType GetEventType() const = 0;
            virtual const char *GetName() const = 0;
            virtual int GetCategoryFlags() const = 0;
            inline bool IsInCategory(EventCategory category)
            {
                return GetCategoryFlags() & category;
            }
        };
        */
    } // namespace Events
} // namespace Myriad

#endif

#include "Events/Event.hpp"
#include "Events/EventContainer.hpp"
#include "Events/EventDispatcher.hpp"
