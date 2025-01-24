#ifndef MYRIAD_CORE_PROFILING_H
#define MYRIAD_CORE_PROFILING_H

#include "core/config.h" //to know what profiling settings we have
#include "core/core.h"

#if MYRIAD_PROFILING == ON
#if MYRIAD_PROFILER == remotery
#include "Remotery.h"
#endif
#endif

namespace Myriad
{
  class MYR_API MyrProfiler
  {
  protected:
#if MYRIAD_PROFILING == ON
#if MYRIAD_PROFILER == remotery
    Remotery *rmt;
#endif
#endif
  public:
    MyrProfiler();
    ~MyrProfiler();
  };
#if MYRIAD_PROFILING == ON
#if MYRIAD_PROFILER == remotery
  extern "C" void
  MyrProfileScoped();
  extern "C" void MyrProfileLog(const char *txt);
#endif
#define PROFILE_SCOPED Myriad::MyrProfileScoped();
#define PROFILE_LOG Myriad::MyrProfileLog
#else
#define PROFILE_SCOPED
#define PROFILE_LOG //
#endif
} // end namespace

#endif
