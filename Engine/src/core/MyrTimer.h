#ifndef MYRIAD_CORE_TIMER_H
#define MYRIAD_CORE_TIMER_H

#include "core/core.h"

namespace Myriad
{
    class MYR_API MyrTimer
    {
      private:
        float time_;
        bool started_;

      public:
        MyrTimer() : time_(0.0f), started_(false) {}
        virtual ~MyrTimer() {}
        void Start() { started_ = true; }
        void Stop() { started_ = false; };
        float Time()
        {
            if (started_)
                time_ += 0.01f;
            return time_;
        }
        void Reset() { time_ = 0.0f; }
    };
} // namespace Myriad
#endif
