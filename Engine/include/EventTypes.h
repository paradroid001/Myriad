#ifndef _MYRIAD_EVENTTYPES_H_
#define _MYRIAD_EVENTTYPES_H_

#include <cstdint>       //uint16_t, uint8_t
#include <functional>    //for std::function in Events
#include <list>          //for Events
#include <queue>         //for MyrIDArray
#include <unordered_map> //for MyrIDArray

#include "EngineConfig.h"
#include "UtilTypes.h" //Logging

namespace Myriad
{
    // Event type is 16 bits wide
    // 8 for primary type
    // 8 for subtype
    typedef uint16_t EventType_t;
    typedef uint8_t EventPrimaryType_t;
    typedef uint8_t EventSubType_t;

    /* Events */
    struct MYR_API EventData
    {
    };

    class MYR_API Event
    {
      protected:
        EventType_t event_type;

      public:
        Event(EventPrimaryType_t ptype, EventSubType_t stype)
            : event_type(ptype << 8 | stype)
        {
        }
        EventPrimaryType_t PType() { return event_type >> 8; }
        EventSubType_t SType() { return event_type & 0x00FF; }
    };

    // I need an untemplated IEventCallback so I can
    // create lists of eventcallback pointers without
    // having to know the template types.
    class MYR_API IEventCallback
    {
      private:
        virtual void Call(Event *) = 0;

      public:
        virtual ~IEventCallback() {}
        void Exec(Event *event) { Call(event); }
    };

    template <typename T> class EventCallback : public IEventCallback
    {
      public:
        ~EventCallback() {}
        // Constructor: pass an instance, and a function
        EventCallback(T *instance,
                      std::function<void(T *, Event *)> member_function)
            : instance_(instance), member_function_(member_function)
        {
        }

        void Call(Event *event) override { member_function_(instance_, event); }

      private:
        T *instance_;
        std::function<void(T *, Event *)> member_function_;
    };

    class MYR_API EventSystem
    {
      protected:
        // Each handler list is a pointer which needs to be allocated.
        std::unordered_map<EventType_t, std::list<IEventCallback *> *>
            subscribers_;

        std::queue<Event *> events;

      public:
        ~EventSystem()
        {
            MYR_TRACE("EventSystem Destructing");
            size_t cleanup_event_counter = 0;
            size_t cleanup_handler_counter = 0;
            size_t cleanup_eventtype_counter = 0;
            // pop and delete all events.
            while (!events.empty())
            {
                Event *e = events.front();
                events.pop();
                delete e;
                cleanup_event_counter += 1;
            }
            // Now cleanup handlers
            for (auto entry : subscribers_)
            {
                // delete each callback
                for (auto callback : *(entry.second))
                {
                    delete callback;
                    cleanup_handler_counter += 1;
                }
                // delete the actual list.
                delete entry.second;
                cleanup_eventtype_counter += 1;
            }
            MYR_TRACE("EventSystem flushed %d events and %d handlers from %d "
                      "eventtype lists",
                      cleanup_event_counter, cleanup_handler_counter,
                      cleanup_eventtype_counter);
        }

        template <typename T>
        void Subscribe(EventPrimaryType_t type, EventSubType_t subtype,
                       T *instance, std::function<void(T *, Event *)> callback)
        {
            EventType_t e_type = type << 8 | subtype;
            // if the key exists..
            if (subscribers_.count(e_type) == 0)
            {
                MYR_TRACE("Created new event type handler list");
                // TODO: could reserve a certain size of list here.
                subscribers_[e_type] = new std::list<IEventCallback *>();
            }
            std::list<IEventCallback *> *handlers = subscribers_[e_type];
            // Add this callback on this instance.
            // TODO: check it already exists?
            handlers->push_back(new EventCallback<T>(instance, callback));
        }

        void Unsubscribe() { MYR_TRACE("Unimplemented"); }

        void Publish(EventPrimaryType_t ptype, EventPrimaryType_t stype,
                     Event *event)
        {
            // synchronously execute all handlers.
            // TODO: call/exec could return a bool to stop propagation.
            // requires all func sigs to change.
            EventType_t e_type = ptype << 8 | stype;
            std::list<IEventCallback *> *handlers = subscribers_[e_type];
            if (handlers != nullptr)
            {
                for (auto handler : *handlers)
                {
                    if (handler != nullptr)
                    {
                        handler->Exec(event);
                    }
                    else // this shouldnt happen
                    {
                        MYR_ERROR("Handler was null.");
                    }
                }
            }
            else
            {
                MYR_ERROR("No subscribers for %d | %d", ptype, stype);
            }
        }

        void AddEvent(Event *event) { events.push(event); }

        void ProcessEvents()
        {
            while (!events.empty())
            {
                Event *e = events.front();
                events.pop();

                // TODO: publish should only need e
                Publish(e->PType(), e->SType(), e);
                // Finished with the event - free.
                delete e;
            }
        }
    };
} // namespace Myriad

#endif
