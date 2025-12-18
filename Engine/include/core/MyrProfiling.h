#ifndef MYRIAD_CORE_PROFILING_H
#define MYRIAD_CORE_PROFILING_H

#include "core/config.h" //to know what profiling settings we have
#include "core/core.h"

#if MYRIAD_PROFILING == PROFILING_ON
#if MYRIAD_PROFILER == PROFILER_REMOTERY
#include "Remotery.h"
#endif
#endif

namespace Myriad
{
  class MYR_API MyrProfiler
  {
  protected:
#if MYRIAD_PROFILING == PROFILING_ON
#if MYRIAD_PROFILER == PROFILER_REMOTERY
    Remotery *rmt;
#endif
#endif
  public:
    MyrProfiler();
    ~MyrProfiler();
  };
#if MYRIAD_PROFILING == PROFILING_ON
#if MYRIAD_PROFILER == PROFILER_REMOTERY
  extern "C" void
  MyrProfileScoped();
  extern "C" void MyrProfileLog(const char *txt);
// #define MyrProfileScoped(name, flags) rmt_ScopedCPUSample(name, flags)
// Just hardcode the 'default' flag (0)
#define MyrProfileScoped(name) rmt_ScopedCPUSample(name, 0)
#define MyrProfileBeginSample(name, flags) rmt_BeginCPUSample(name, flags)
#define MyrProfileEndSample rmt_EndCPUSample

#else
  // #define PROFILE_SCOPED Myriad::MyrProfileScoped();
  // #define PROFILE_LOG Myriad::MyrProfileLog

#define PROFILE_SCOPED
#define PROFILE_LOG //
#endif
#else
// Define to nothing?
#define MyrProfileScoped(name)
#define MyrProfileBeginSample(name, flags)
#define MyrProfileEndSample
#endif
} // end namespace

#endif
