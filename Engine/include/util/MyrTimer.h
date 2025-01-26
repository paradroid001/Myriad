#ifndef MYRIAD_UTIL_TIMER_H
#define MYRIAD_UTIL_TIMER_H

#include "core/core.h"

namespace Myriad
{
  class MYR_API MyrTimer
  {
  private:
    std::chrono::high_resolution_clock::time_point start_time_;
    std::chrono::high_resolution_clock::time_point end_time_;
    uint32_t elapsed_;
    bool started_;

  public:
    MyrTimer() {}
    virtual ~MyrTimer() {}
    void Start()
    {
      started_ = true;
      Reset();
    }
    void Stop()
    {
      started_ = false;
      end_time_ = std::chrono::high_resolution_clock::now();
      elapsed_ = std::chrono::duration_cast<std::chrono::milliseconds>(end_time_ - start_time_).count();
    };

    uint32_t Time()
    {
      if (started_)
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time_).count();
      else
        return 0;
    }
    uint32_t Elapsed() const
    {
      return elapsed_;
    }
    void Reset()
    {
      start_time_ = std::chrono::high_resolution_clock::now();
    }
  };
} // namespace Myriad
#endif
