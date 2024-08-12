#include "core/event/MyrEventService.h"
#include "io/Log.h"

namespace Myriad
{
    MyrEventService::~MyrEventService()
    {
        delete dispatcher_;
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
} // namespace Myriad
