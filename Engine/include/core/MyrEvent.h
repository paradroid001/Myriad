#ifndef MYRIAD_CORE_MYREVENT_H
#define MYRIAD_CORE_MYREVENT_H

#include "core/core.h"
#include "core/config.h"
#include <list>
#include <map>
#include <functional>
#include <vector>

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
  template <class T>
  class EventCallback : public IEventCallback
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
    void Subscribe(MYREVENT_PRIMARYTYPE_t type, MYREVENT_SUBTYPE_t subtype, T *instance,
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
    void Publish(MYREVENT_PRIMARYTYPE_t type, MYREVENT_SUBTYPE_t subtype, MyrEvent *p_event)
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
    inline static MyrEventService *p_event_service_ = nullptr;

  protected:
    MYREVENT_PRIMARYTYPE_t type_;
    MYREVENT_SUBTYPE_t subtype_;

  public:
    MyrEvent(MYREVENT_PRIMARYTYPE_t type, MYREVENT_SUBTYPE_t subtype) : type_(type), subtype_(subtype)
    {
    }

    static void SetEventService(MyrEventService *p_service)
    {
      MyrEvent::p_event_service_ = p_service;
    }

    static MyrEventService *GetEventService()
    {
      return MyrEvent::p_event_service_;
    }

    MYREVENT_PRIMARYTYPE_t GetType() const { return type_; }
    MYREVENT_SUBTYPE_t GetSubType() const { return subtype_; }
    MYREVENT_TYPE_t GetFullType() const { return type_ << 8 | subtype_; }

    template <class T>
    static void Register(uint8_t type, uint8_t subtype, T *instance,
                         std::function<void(T *, MyrEvent *)> callback)
    {
      // Instead of a passed one, we are going to use the Event Service
      // p_dispatcher->Subscribe(type, subtype, instance, callback);
      MyrEvent::p_event_service_->GetDispatcher()->Subscribe(
          type, subtype, instance, callback);
    }
    void Unregister();
    void Emit()
    {
      // Instead of using a passed event dispatcher to publish the
      // event... e->Publish(type_, subtype_, this); We queue the event in
      // the event service instead.
      MyrEvent::p_event_service_->AddEvent(this);
    }
    virtual ~MyrEvent() {}
  };
}

#endif
