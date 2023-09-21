#pragma once
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