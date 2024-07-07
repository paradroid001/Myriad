#ifndef MYRIAD_CORE_THREAD_IJOB_H
#define MYRIAD_CORE_THREAD_IJOB_H

#include <functional>

// A job is an abstraction of some work we want to do.
// It may have dependencies (IJobs it is waiting on)
// It may have dependents (IJobs waiting on it)

namespace Myriad
{
    class IJob
    {
      protected:
        std::string name;
        std::function<void()> jobfn;

      public:
        IJob(std::string job_name, std::function<void()> job_function)
            : name(job_name), jobfn(job_function)
        {
        }
        virtual ~IJob(){};
        virtual const char *GetName() { return name.c_str(); }
        virtual std::function<void()> GetTask() { return jobfn; }
    };
} // namespace Myriad
#endif
