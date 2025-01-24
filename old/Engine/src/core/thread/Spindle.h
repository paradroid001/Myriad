#ifndef MYRIAD_CORE_THREAD_SPINDLE_H
#define MYRIAD_CORE_THREAD_SPINDLE_H

#include <list>

#include "core/core.h"
#include "core/thread/ScheduledJob.h"

namespace Myriad
{

    const int SPINDLE_INVALID = 255;
    // Spindle encapsulates a thread and some state
    // for that thread
    class MYR_API Spindle
    {
      protected:
        uint8_t spindle_id;
        // mutex - lock the spindle with this
        // std::mutex spindle_mutex_;
        // a list because we might pull stuff out of the middle
        std::list<ScheduledJob *> job_list;
        // std::thread *p_job_thread;
        bool stop = false;

      public:
        // Opt into the move constructor
        // Spindle(Spindle &&) = default;

        // Custom move constructor because std::thread is not copyable.
        /*
        Spindle(Spindle &&other)
            : spindle_id(other.spindle_id),
              job_thread(std::move(other.job_thread))
        {
        }
        */
        Spindle(uint8_t id) : spindle_id(id), stop(false) //,
        {
            MYR_CORE_INFO("Finished constructing Spindle {0}.", spindle_id);
        }

        ~Spindle()
        {
            MYR_CORE_ERROR("Spindle {0} destructor is signalling current "
                           "thread to finish.",
                           spindle_id);
            stop = true;
        }

        uint8_t GetID() { return spindle_id; }
        std::list<ScheduledJob *> GetJobList() { return job_list; }
        void AddJob(ScheduledJob *job) { job_list.push_back(job); }

        // The task of spindlespinner is to grab the
        // next ready job off the list, and execute it.
        void SpindleSpinner()
        {
            auto total_time = 0;
            auto milliseconds = 200;
            while (!stop)
            {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(milliseconds));
                total_time += milliseconds;
                MYR_TRACE("Spindle {0} is alive: {1}", spindle_id, total_time);
            }
        }

        void Stop() { stop = true; }
    };
} // namespace Myriad
#endif
