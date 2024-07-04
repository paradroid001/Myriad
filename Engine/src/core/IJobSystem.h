#ifndef _MYRIAD_CORE_IJOBSYSTEM_H_
#define _MYRIAD_CORE_IJOBSYSTEM_H_

#include "Job.h"
#include "core.h"

namespace MYR_API Myriad
{
    class IJobSystem
    {
      public:
        virtual ~IJobSystem(){};
        virtual void Init() = 0;
        virtual void Drain() = 0;
        virtual void AddJob(Job *job) = 0;
    };
} // namespace MYR_API Myriad
#endif
