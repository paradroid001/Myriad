#ifndef MYRIAD_CORE_SCHEDULEDJOB_H
#define MYRIAD_CORE_SCHEDULEDJOB_H

#include <functional>
#include <vector>

#include "core/core.h"
#include "io/Log.h"

namespace Myriad
{
    /*
      A scheduled job should be able to:
      - be able to be reset/rerun (reused!)
      - tell the scheduler its dependencies.
      - tell the scheduler it is blocked waiting for deps
      - tell other jobs that it has finished its work
      - spawn other jobs (stretch?)

      The main workflow is:
      // BlockIfDependenciesArePending() (not actually block, but swap in other
      work)
      // Do work
      // SignalDependentsImDone

    */
    class MYR_API ScheduledJob
    {
      protected:
        std::string name;
        uint8_t assigned_spindle_id; // which spindle have we been put on?
        std::vector<ScheduledJob> dependents; // jobs waiting for me.
        size_t dependencies_remaining;        // how many am I waiting for.
        // void ExecuteJob() { Execute(); };
        //  the function that will be excecuted.
        // virtual void Execute() {} // Client classes override
        std::function<void()> jobfn_;

      public:
        ScheduledJob(const char *job_name, std::function<void()> job_function)
        {
            name = job_name;
            jobfn_ = job_function;
            assigned_spindle_id = 255; // SPINDLE_INVALID; //TODO
        }

        ~ScheduledJob()
        {
            // TODO
            //  This is to be determined.
            MYR_CORE_WARN("Scheduled Job {0} is destructing", name);
        }
        const char *GetName() { return name.c_str(); }
        uint8_t GetSpindleID() { return assigned_spindle_id; }
        void SetSpindleID(uint8_t id)
        {
            if (id == 255) // SPINDLE_INVALID) //TODO
            {
                MYR_ERROR("Assigned invalid spindle id to job {0}", name);
            }
            assigned_spindle_id = id;
        }

        std::function<void()> GetTask() { return jobfn_; }
    };
} // namespace Myriad
#endif
