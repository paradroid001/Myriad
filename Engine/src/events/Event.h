#ifndef __EVENT_H_
#define __EVENT_H_

#include "core/core.h"
#include <functional>
#include <list>
#include <map>
#include <typeindex>
#include <typeinfo>

namespace Myriad
{
    namespace Events
    {
        class Event; // fwd declare

        class MYR_API IEventCallback
        {
          public:
            void exec(Event *evnt) { call(evnt); }

          private:
            virtual void call(Event *evnt) = 0;
        };

        // may not need dllexport here..
        template <class T, class EventType>
        class EventCallback : public IEventCallback
        {
          public:
            typedef void (T::*MemberFunction)(EventType *);
            EventCallback(T *instance, MemberFunction memberFunction)
                : instance{instance}, memberFunction{memberFunction} {};

            void call(Event *evnt)
            {
                // cast to correct type and call member fn
                (instance->*memberFunction)(static_cast<EventType *>(evnt));
            }

          private:
            // ptr to instance
            T *instance;
            // prt to member function (typedef is a pointer)
            MemberFunction memberFunction;
        };

        typedef std::list<IEventCallback *> HandlerList;

        class MYR_API EventDispatcher
        {
          public:
            static EventDispatcher *Instance()
            {
                if (_instance == NULL)
                {
                    _instance = new EventDispatcher();
                }
                return _instance;
            }

            template <typename EventType> void publish(EventType *evnt)
            {
                // Here, instead of checking typeid(EventType), we
                // check the typeid of the dereferenced event pointer, *evnt.
                // This is because we call publish from the base Event class
                // Emit(), so EventType is always Event/PEvent. Dereferencing
                // the pointer forces evaluation of the polymorphic type.
                // Was: = subscribers[typeid(EventType)];
                HandlerList *handlers = subscribers[typeid(*evnt)];

                if (handlers == nullptr)
                {
                    return;
                }

                for (auto &handler : *handlers)
                {
                    if (handler != nullptr)
                    {
                        handler->exec(evnt);
                    }
                }
            }

            template <class T, class EventType>
            void subscribe(T *instance, void (T::*memberFunction)(EventType *))
            {
                HandlerList *handlers = subscribers[typeid(EventType)];

                // First time initialization
                if (handlers == nullptr)
                {
                    handlers = new HandlerList();
                    // std::cout << "Subscriber Type was "
                    //           << typeid(EventType).name() << std::endl;
                    // std::cout << "Subscriber Hash was "
                    //           << typeid(EventType).hash_code() << std::endl;
                    subscribers[typeid(EventType)] = handlers;
                }

                handlers->push_back(
                    new EventCallback<T, EventType>(instance, memberFunction));
            }

          private:
            inline static /*__declspec(dllexport)*/ EventDispatcher *_instance;
            EventDispatcher(){};
            std::map<std::type_index, HandlerList *> subscribers;
        };

        class MYR_API Event //: public IEvent
        {
          public:
            template <class T, class EventType>
            static void Register(T *instance,
                                 void (T::*memberFunction)(EventType *))
            {
                EventDispatcher::Instance()->subscribe(instance,
                                                       memberFunction);
            };
            static void Unregister(){};
            void Emit() { EventDispatcher::Instance()->publish(this); };

          protected:
            virtual ~Event(){};
        };
    } // namespace Events
} // namespace Myriad

#endif
