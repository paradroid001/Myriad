#ifndef _MYRIAD_CORE_JOB_H_
#define _MYRIAD_CORE_JOB_H_

#include "core.h"
#include "io/Log.h"
#include <functional>

namespace Myriad
{
    class MYR_API Job
    {
      protected:
        std::string job_name;
        void RunTask() { Execute(); };
        // the function that will be excecuted.
        virtual void Execute() = 0; // { MYR_CORE_INFO("Inside execute"); };

      public:
        Job(const char *name) { job_name = name; }
        ~Job() { MYR_CORE_WARN("Job {0} is destructing.", job_name); }
        const char *GetName() { return job_name.c_str(); }
        std::function<void()> GetTask()
        {
            return std::bind(&Job::RunTask, this);
        } // using () operator
        // void operator()() { Execute(); };
    };
} // namespace Myriad
#endif
