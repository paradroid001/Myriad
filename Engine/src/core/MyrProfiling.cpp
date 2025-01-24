#include "core/MyrProfiling.h"
#include "io/MyrLogging.h"
#include "core/config.h"

namespace Myriad
{
  MyrProfiler::MyrProfiler()
  {
#if MYRIAD_PROFILING == 1
#if MYRIAD_PROFILER == 1
    rmt_CreateGlobalInstance(&rmt);
    MYR_CORE_INFO("Created Remotery profiler");
#endif
#endif
  }
  MyrProfiler::~MyrProfiler()
  {
#if MYRIAD_PROFILING == 1
#if MYRIAD_PROFILER == 1
    rmt_DestroyGlobalInstance(rmt);
    MYR_CORE_INFO("Destroyed Remotery profiler");
#endif
#endif
  }

#if MYRIAD_PROFILING == 1
#if MYRIAD_PROFILER == 1
  void MyrProfileScoped()
  {
    rmt_ScopedCPUSample(LogText, 0);
  }
  void MyrProfileLog(const char *txt)
  {
    rmt_LogText(txt);
    rmt_LogText("kfjdskjfkdsjl");
  }
#endif
#else
  void MyrProfileScoped() {}
  void MyrProfileLog(const char *txt) {}
#endif
} // end namespace
