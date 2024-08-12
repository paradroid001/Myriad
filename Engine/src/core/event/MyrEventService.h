#ifndef MYRIAD_EVEMT_MYREVENTSERVICE
#define MYRIAD_EVEMT_MYREVENTSERVICE

#include "core/IService.h"
#include "core/core.h"
#include "core/event/MyrEvent.h"
#include <vector>

namespace Myriad
{
    class MYR_API MyrEventService : public IService
    {
      private:
        EventDispatcher *dispatcher_;
        std::vector<MyrEvent *> events;

      protected:
        bool WhenStartService() override;
        bool WhenStopService() override;

      public:
        ~MyrEventService();
    };
} // namespace Myriad

#endif
