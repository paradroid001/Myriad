#ifndef _MYRIAD_CORE_IJOBSYSTEM_H_
#define _MYRIAD_CORE_IJOBSYSTEM_H_

#include "Job.h"
#include "core.h"

namespace Myriad
{
    class MYR_API IJobSystem
    {
      public:
        virtual ~IJobSystem(){};
        virtual void Init() = 0;
        virtual void Drain() = 0;
        virtual void AddJob(Job &job) = 0;
    };
} // namespace Myriad
#endif
