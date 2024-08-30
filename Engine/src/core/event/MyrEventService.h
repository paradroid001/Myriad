#ifndef MYRIAD_EVEMT_MYREVENTSERVICE
#define MYRIAD_EVEMT_MYREVENTSERVICE

#include "core/IService.h"
#include "core/MyrSingleton.h"
#include "core/core.h"
// #include "core/event/MyrEvent.h"
#include <vector>

namespace Myriad
{
    // Fwd declare MyrEvent and EventDispatcher
    class MyrEvent;
    class EventDispatcher;

    class MYR_API MyrEventService : public IService,
                                    public MyrSingleton<MyrEventService>
    {

        // This using line prevents errors finding
        // the myrsingleton constructor..
        using MyrSingleton<MyrEventService>::MyrSingleton;

      private:
        EventDispatcher *dispatcher_;
        std::vector<MyrEvent *> events_;

      protected:
        bool WhenStartService() override;
        bool WhenStopService() override;

      public:
        ~MyrEventService();
        EventDispatcher *GetDispatcher() const { return dispatcher_; }
        void AddEvent(MyrEvent *p_event);
        void ProcessEvents();
        void ClearEvents();
    };
} // namespace Myriad

#endif
