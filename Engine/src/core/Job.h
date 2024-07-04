#ifndef _MYRIAD_CORE_JOB_H_
#define _MYRIAD_CORE_JOB_H_

#include "core.h"
#include "io/Log.h"
#include <functional>

namespace Myriad
{
    class MYR_API Job
    {

      public:
        ~Job(){};
        std::function<void()> GetTask()
        {
            return std::bind(&Job::RunTask, this);
        }; // using () operator
        // void operator()() { Execute(); };

      protected:
        void RunTask() { Execute(); };
        // the function that will be excecuted.
        virtual void Execute() = 0; // { MYR_CORE_INFO("Inside execute"); };
    };
} // namespace Myriad
#endif
