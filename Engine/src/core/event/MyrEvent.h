#ifndef MYRIAD_CORE_EVENT_MYREVENT_H
#define MYRIAD_CORE_EVENT_MYREVENT_H

#include "core/core.h"
#include <cstdint>
#include <functional>
#include <list>
#include <map>

namespace Myriad
{
    typedef enum MyrEventType
    {
        MYR_EVENT_NONE = 0,
        MYR_EVENT_TIMER = 1,            // timers
        MYR_EVENT_SYSTEM = 2,           // close window, resize, hot reload, etc
        MYR_EVENT_INPUT = 4,            // keypress, mouse, etc
        MYR_EVENT_OBJECT = 8,           // Object destroy, creation, etc
        MYR_EVENT_FRAME_LIFECYCLE = 16, // on frameX, on renderX, on updateX
        MYR_EVENT_USER = 32,            // user defined events in user scripts.
        MYR_EVENT_CUSTOM = 64           // custom events (?)
    } MyrEventType;

    class MyrEvent; // fwd declare

    class MYR_API IEventCallback
    {
      public:
        // TODO should this return bool (the event is handled...)
        void exec(MyrEvent *p_event) { Call(p_event); }

      private:
        virtual void Call(MyrEvent *p_event) = 0;
    };

    // Ok, I've just realised why I need a callback<T> class,
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
        // A dict of uint16 to handler list *
        std::map<uint16_t, HandlerList *> subscribers_;

      public:
        template <class T>
        void Subscribe(uint8_t type, uint8_t subtype, T *instance,
                       std::function<void(T *, MyrEvent *)> callback)
        {
            uint16_t key = type << 8 | subtype;
            HandlerList *p_handlers = subscribers_[key];
            if (p_handlers == nullptr)
            {
                p_handlers = new HandlerList();
                subscribers_[key] = p_handlers;
            }

            p_handlers->push_back(new EventCallback<T>(instance, callback));
        }

        void Unsubscribe() {};
        void Publish(uint8_t type, uint8_t subtype, MyrEvent *p_event)
        {
            uint16_t key = type << 8 | subtype;
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

      protected:
        uint8_t type_;
        uint8_t subtype_;

      public:
        MyrEvent(uint8_t type, uint8_t subtype) : type_(type), subtype_(subtype)
        {
        }

        uint8_t GetType() const { return type_; }
        uint8_t GetSubType() const { return subtype_; }
        uint16_t GetFullType() const { return type_ << 8 | subtype_; }

        template <class T>
        static void Register(uint8_t type, uint8_t subtype, T *instance,
                             std::function<void(T *, MyrEvent *)> callback,
                             EventDispatcher *p_dispatcher)
        {
            p_dispatcher->Subscribe(type, subtype, instance, callback);
        }
        void Unregister();
        void Emit(EventDispatcher *e) { e->Publish(type_, subtype_, this); }
        virtual ~MyrEvent() {}
    };
} // namespace Myriad

/* How do we want to use events?
  --User code:
    MyrEvent e = new MyrEvent(TYPE, SUBTYPE);
    e.SetData(...);
    s.Emit();
    //but howw was this registered?

    //Better:
    class MyEvent : public MyrEvent
    {
      public:
        MyEvent() : MyrEvent(SOMETYPE, SOMESUBTYPE)
        int field1;
        float field2;
    }

    class MyObjectClass
    {
      public:
        void MyCallback(MyEvent& e)
        {
          ...
        }
    }

    p_myobj = new MyObjectClass();
    MyEvent::Register(p_myobj, &MyObjectClass::MyCallback);

    MyEvent *t = new MyEvent();
    t->field1 = 10;
    t->field2 = 10.0f;

    t->Emit(); //make the event propagate.

*/

#endif
