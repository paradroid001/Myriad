#ifndef _MYRIAD_CORE_IJOBSYSTEM_H_
#define _MYRIAD_CORE_IJOBSYSTEM_H_

// #include "core/memory/MyrHandle.h"
#include "core/core.h"
#include "core/thread/IJob.h"

namespace Myriad
{
    class MYR_API IJobSystem
    {
      public:
        virtual ~IJobSystem(){};
        // virtual void Init() = 0;
        virtual void Submit(IJob *job) = 0;
        virtual void Drain() = 0;
        virtual bool IsBusy() = 0; // Is this actively running jobs?
        virtual void Wait() = 0;   // Wait (block) until this is idle.
    };
} // namespace Myriad
#endif
