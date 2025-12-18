#include "core/MyrProfiling.h"
#include "io/MyrLogging.h"
#include "core/config.h"

namespace Myriad
{
  MyrProfiler::MyrProfiler()
  {
#if MYRIAD_PROFILING == PROFILING_ON
#if MYRIAD_PROFILER == PROFILER_REMOTERY
    rmt_CreateGlobalInstance(&rmt);
    MYR_CORE_INFO("Created Remotery profiler");
#endif
#endif
  }
  MyrProfiler::~MyrProfiler()
  {
#if MYRIAD_PROFILING == PROFILING_ON
#if MYRIAD_PROFILER == PROFILER_REMOTERY
    rmt_DestroyGlobalInstance(rmt);
    MYR_CORE_INFO("Destroyed Remotery profiler");
#endif
#endif
  }

#if MYRIAD_PROFILING == PROFILING_ON
#if MYRIAD_PROFILER == PROFILER_REMOTERY

  /*
  void MyrProfileScoped()
  {
    rmt_ScopedCPUSample(MyriadTestName, 0);
  }

  void MyrProfileLog(const char *txt)
  {
    rmt_LogText(txt);
    rmt_LogText("kfjdskjfkdsjl");
  }
  */
#endif
#else
  // void MyrProfileScoped() {}
  // void MyrProfileLog(const char *txt) {}
#endif
} // end namespace
