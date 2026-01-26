#ifndef _TEST_EVENTS_H_
#define _TEST_EVENTS_H_

#include <map>
#include <functional>
#include <list>
#include <vector>

#include <iostream>

namespace events
{
  class EventDispatcher; // fwd

  class IEvent
  {
  public:
    virtual ~IEvent() {};
    virtual void Emit(EventDispatcher &dispatcher) = 0;
  };

  // typedef std::function<bool(Event &e)> event_handler_t;
  // typedef std::list<event_handler_t> handlers_t;

  struct IFunctor
  {
  protected:
    virtual void call(IEvent *event) = 0;

  public:
    void operator()(IEvent *event)
    {
      call(event);
    }
    virtual ~IFunctor() {};
  };

  // typedef std::function<void(RLBehaviour *, RLEvent *)> rl_handler_t;

  template <typename T>
  struct EventFunctor : public IFunctor
  {
  protected:
    T *instance;
    std::function<void(T *, IEvent *)> func;

    void call(IEvent *event) override
    {
      func(instance, event);
    }

  public:
    EventFunctor(T *instance, std::function<void(T *, IEvent *)> f) : instance(instance), func(f) {}
    virtual ~EventFunctor() {};
  };

  class EventDispatcher
  {
  protected:
    std::map<int, std::vector<IFunctor *>> handlers;

  public:
    EventDispatcher() {}
    ~EventDispatcher() {}
    template <typename T, typename U>
    // bool Subscribe(int event_type, T *instance, event_handler_t &func)
    bool Subscribe(int type, U *instance, std::function<void(U *, IEvent *)> fn)
    {
      handlers[static_cast<int>(type)].push_back(new EventFunctor<U>(instance, fn));

      // handlers[static_cast<int>(type)].emplace_back<EventFunctor<U>>(fn);
      return true;
    }
    template <typename T>
    bool Call(int event_type, T *event)
    {
      if (handlers.count(event_type) != 0)
      {
        for (auto it = handlers[event_type].begin(); it != handlers[event_type].end(); ++it)
        {
          (**it)(event);
        }
      }
      else
      {
        MYR_TRACE("Dispatcher: no match for event type {0}", event_type);
      }
      return true;
    }
  };

  class EventSystem
  {
  protected:
    EventDispatcher dispatcher;

  public:
    EventSystem() {}
    ~EventSystem() {}
    EventDispatcher &GetDispatcher() { return dispatcher; }
    /*
    template <typename U>
    void
    Register(int event_type, U *instance, std::function<bool(U *, Event *)> callback)
    {
      dispatcher.Subscribe(event_type, instance, callback);
    }
    template <typename U>
    bool Publish(int event_type, U &event)
    {
      dispatcher.Call(event_type, event);
      return true;
    }
      */
  };

  template <typename T>
  class Event : public IEvent
  {
  private:
    Event() : event_system(nullptr) {}

  protected:
    T event_type;
    EventSystem *event_system; // the es we are registered with.
    T GetEventType() { return event_type; }

  public:
    // This is the only valid constructor
    Event(EventSystem *es) : event_system(es) {}
    virtual ~Event() {};
    EventSystem *GetEventSystem() { return event_system; }

    virtual void Emit(EventDispatcher &dispatcher) override
    {
      dispatcher.Call(event_type, this);
    }

    /*
    template <typename T>
    static void Emit(T &event)
    {
      event.GetEventSystem()->Publish<T>(event.GetEventType(), event);
    }
    */

    // void Send() {}
    // void Broadcast() {}
  };

}
#endif
