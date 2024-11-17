#include "core/event/MyrEventService.h"
#include "core/event/MyrEvent.h"
#include "io/Log.h"

namespace Myriad
{
    MyrEventService::~MyrEventService()
    {
        if (dispatcher_ != nullptr)
        {
            delete dispatcher_;
        }
        MYR_CORE_INFO("MyrEventService Destructing");
    }

    bool MyrEventService::WhenStartService()
    {
        MYR_CORE_INFO("MyrEventService Started");
        dispatcher_ = new EventDispatcher();
        return true;
    }
    bool MyrEventService::WhenStopService()
    {
        MYR_CORE_INFO("MyrEventService Stopped");
        return true;
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
    void MyrEventService::ClearEvents() { events_.clear(); }
} // namespace Myriad
