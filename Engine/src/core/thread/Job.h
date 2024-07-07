#ifndef _MYRIAD_CORE_JOB_H_
#define _MYRIAD_CORE_JOB_H_

#include "core/core.h"
#include "core/thread/IJob.h"
#include "io/Log.h"
#include <functional>

namespace Myriad
{
    class MYR_API Job : public IJob
    {
      protected:
        void RunTask() { Execute(); };
        // the function that will be excecuted.
        virtual void Execute() = 0; // { MYR_CORE_INFO("Inside execute"); };

      public:
        Job(std::string job_name)
            : IJob(job_name, std::bind(&Job::RunTask, this))
        {
        }
        ~Job() { MYR_CORE_WARN("Job {0} is destructing.", name); }
        // std::function<void()> GetTask()
        //{
        //     return std::bind(&Job::RunTask, this);
        // } // using () operator
        //  void operator()() { Execute(); };
    };
} // namespace Myriad
#endif
