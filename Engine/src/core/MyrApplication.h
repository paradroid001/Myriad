#ifndef MYRIAD_CORE_MYRAPPLICATION_H
#define MYRIAD_CORE_MYRAPPLICATION_H

#include "core.h"

namespace Myriad
{
    class MYR_API MyrApplication
    {
      public:
        MyrApplication();
        virtual ~MyrApplication();
        virtual void Run();
    };

    // Clients of the shared library use this to create their application.
    MyrApplication *CreateApplication();

} // namespace Myriad

#endif