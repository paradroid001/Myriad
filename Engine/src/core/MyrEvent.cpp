#include "core/MyrEvent.h"
#include "io/MyrLogging.h"

namespace Myriad
{
  MyrEventService::MyrEventService()
  {
    MYR_CORE_INFO("MyrEventService Started");
    dispatcher_ = new EventDispatcher();
  }
  MyrEventService::~MyrEventService()
  {
    if (dispatcher_ != nullptr)
    {
      delete dispatcher_;
    }
    MYR_CORE_INFO("MyrEventService Destructing");
  }
  void MyrEventService::AddEvent(MyrEvent *p_event)
  {
    events_.push_back(p_event);
  }
  void MyrEventService::ProcessEvents()
  {
    // TODO: so we pushed a bunch of event pointers.
    // but they could be gone now...?
    // Yikes.
    for (auto *p_event : events_)
    {
      dispatcher_->Publish(p_event->GetType(), p_event->GetSubType(),
                           p_event);
    }
  }

  void MyrEventService::ClearEvents()
  {
    // TODO: either these events need to be deleted OR
    // the event manager holds an event pool and reuses them.
    events_.clear();
  }

}
