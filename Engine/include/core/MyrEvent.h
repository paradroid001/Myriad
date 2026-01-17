#ifndef MYRIAD_CORE_MYREVENT_H
#define MYRIAD_CORE_MYREVENT_H

#include "core/config.h"
#include "core/core.h"
#include <functional>
#include <list>
#include <map>
#include <unordered_map>
#include <vector>

#include "io/MyrLogging.h"

namespace Myriad
{
    class MyrEvent; // fwd declare
    class EventDispatcher;

    class MYR_API MyrEventService
    {
      private:
        EventDispatcher *dispatcher_;
        std::vector<MyrEvent *> events_;

      public:
        MyrEventService();
        ~MyrEventService();
        EventDispatcher *GetDispatcher() const { return dispatcher_; }
        void AddEvent(MyrEvent *p_event);
        void ProcessEvents();
        void ClearEvents();
    };

    class MYR_API IEventCallback
    {
      public:
        void exec(MyrEvent *p_event) { Call(p_event); }

      private:
        virtual void Call(MyrEvent *p_event) = 0;
    };

    // We need a callback<T> class,
    // because the first arg to the function actually needs to be
    // an instance - a T*, otherwise you can't call T->function
    template <class T> class EventCallback : public IEventCallback
    {
      public:
        EventCallback(T *instance,
                      std::function<void(T *, MyrEvent *)> member_function)
            : instance_(instance), member_function_(member_function)
        {
        }

        void Call(MyrEvent *p_event) { member_function_(instance_, p_event); }

      private:
        T *instance_;
        std::function<void(T *, MyrEvent *)> member_function_;
    };

    // typedef std::list<std::function<void(MyrEvent *)>>
    typedef std::list<IEventCallback *> HandlerList;

    // This class holds the registered callbacks for each event type/subtype.
    class MYR_API EventDispatcher
    {
      private:
        // A dict of MYREVENT_TYPE_t to handler list *
        std::map<MYREVENT_TYPE_t, HandlerList *> subscribers_;

      public:
        template <class T>
        void Subscribe(MYREVENT_PRIMARYTYPE_t type, MYREVENT_SUBTYPE_t subtype,
                       T *instance,
                       std::function<void(T *, MyrEvent *)> callback)
        {
            MYREVENT_TYPE_t key = type << 8 | subtype;
            HandlerList *p_handlers = subscribers_[key];
            if (p_handlers == nullptr)
            {
                p_handlers = new HandlerList();
                subscribers_[key] = p_handlers;
            }

            p_handlers->push_back(new EventCallback<T>(instance, callback));
        }

        void Unsubscribe() {};

        // Publish immediately / synchronously executes callbacks
        // on all obseervers.
        void Publish(MYREVENT_PRIMARYTYPE_t type, MYREVENT_SUBTYPE_t subtype,
                     MyrEvent *p_event)
        {
            MYREVENT_TYPE_t key = type << 8 | subtype;
            HandlerList *p_handlers = subscribers_[key];
            if (p_handlers != nullptr)
            {
                for (auto &handler : *p_handlers)
                {
                    if (handler != nullptr)
                    {
                        handler->exec(p_event);
                    }
                }
            }
        }
    };

    class MYR_API MyrEvent
    {
      private:
        MyrEvent() { /*should not be callable*/ }

        // The event router manages which event system handles certain types of
        // events. You could get fancy with this and have multiple event
        // services, but lets just keep it simple.
        // inline just keeps us from needing a cpp file for this.
        inline static std::unordered_map<MYREVENT_TYPE_t, MyrEventService *>
            router;

        // This is here for historical reasons when I thought there
        // would only ever be one event service. It can be thought of
        // as the 'default' route when nothing matches.
        // inline just keeps us from needing a cpp file for this.
        inline static MyrEventService *p_event_service_ = nullptr;

      protected:
        // The type and subtype of the event.
        MYREVENT_PRIMARYTYPE_t type_;
        MYREVENT_SUBTYPE_t subtype_;

      public:
        MyrEvent(MYREVENT_PRIMARYTYPE_t type, MYREVENT_SUBTYPE_t subtype)
            : type_(type), subtype_(subtype)
        {
        }

        /**
         * TODO / HACK/ : this is really just a default
         */
        static void SetEventService(MyrEventService *p_service)
        {
            MyrEvent::p_event_service_ = p_service;
        }

        // This shpould be thought of as 'get default event service'
        static MyrEventService *GetEventService()
        {
            return MyrEvent::p_event_service_;
        }

        MYREVENT_PRIMARYTYPE_t GetType() const { return type_; }
        MYREVENT_SUBTYPE_t GetSubType() const { return subtype_; }
        // TODO: I don't love how these two functions (the instance and static
        // versions)
        //  are called the same thing. BUt they are both const and do the same
        //  thing so maybe it's ok?
        inline static MYREVENT_TYPE_t GetFullType(MYREVENT_PRIMARYTYPE_t type,
                                                  MYREVENT_SUBTYPE_t subtype)
        {
            return type << 8 | subtype;
        }
        MYREVENT_TYPE_t GetFullType() const
        {
            return MyrEvent::GetFullType(type_, subtype_);
        }

        // This function registers to the default event service
        template <class T>
        static void Register(uint8_t type, uint8_t subtype, T *instance,
                             std::function<void(T *, MyrEvent *)> callback)
        {
            // Instead of a passed one, we are going to use the Event Service
            // p_dispatcher->Subscribe(type, subtype, instance, callback);
            MyrEvent::p_event_service_->GetDispatcher()->Subscribe(
                type, subtype, instance, callback);
        }

        // This function routes events of a certain type (type|subtype)
        // to a particular event service, and registers the handler.
        template <class T>
        static void Route(MyrEventService *service, uint8_t type,
                          uint8_t subtype, T *instance,
                          std::function<void(T *, MyrEvent *)> callback)
        {
            if (nullptr == service)
            {
                MYR_CORE_ERROR("Routing event to null service: abort");
                return;
            }
            if (nullptr == instance)
            {
                MYR_CORE_ERROR("Routing event to null object: abort.");
            }

            MYREVENT_TYPE_t fulltype = GetFullType(type, subtype);
            // count() counts the occurrences of the key.
            // In C++20 we could use contains() but...
            if (router.count(fulltype))
            {
                // found
                if (router[fulltype] != service)
                {
                    MYR_CORE_WARN("Attempt to route event [{0}|{1}] to another "
                                  "service: doing nothing.",
                                  type, subtype);
                }
                // Otherwise this event type was already routed to
                //  this service. Also do nothing.
            }
            else
            {
                // not found. Add it.
                router[fulltype] = service;
            }

            // Actually subscribe
            service->GetDispatcher()->Subscribe(type, subtype, instance,
                                                callback);
        }

        void Unregister();

        void Emit()
        {
            // If we find the event type in the router, use that.
            // Otherwise, we use the default dispacher.
            MyrEventService *service = MyrEvent::p_event_service_;

            // Find the type
            const auto &element = MyrEvent::router.find(GetFullType());
            if (element != MyrEvent::router.end())
            {
                // key was found
                service = element->second;
            }
            // Add to the event service queue.
            service->AddEvent(this);
        }
        virtual ~MyrEvent() {}
    };
} // namespace Myriad

#endif
