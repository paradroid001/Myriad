#ifndef MYRIAD_CORE_THREADPOOLJOBSYSTEM_H
#define MYRIAD_CORE_THREADPOOLJOBSYSTEM_H

#include <list>
#include <map>
#include <queue>
#include <thread>
#include <vector>

#include "core/thread/ScheduledJob.h"
#include "core/thread/Spindle.h"

namespace Myriad
{

    // Creates Spindles (max uint8_t)
    // Jobs can be submitted to a global structure which
    // respects dependencies.
    // The dependency jobs must already be present in the system.
    // The scheduler then allocates to the appropriate Spindle.
    class MYR_API ThreadPoolJobSystem
    {
      protected:
        // The queue of incoming jobs
        std::queue<ScheduledJob *> incoming_jobs;
        //  A map of jobs to the spindle id they are assigned to.
        std::unordered_map<ScheduledJob *, uint8_t> known_jobs_map;
        std::vector<Spindle> spindles;
        // better for this to own them, or
        // it's a world of pain inside Spindle.
        std::vector<std::thread> threads;
        std::uint8_t next_spindle_index = 0;

        // Allocate a job to a spindle.
        // If spindle preference is null, they don't care which.
        uint8_t AllocateToSpindle(ScheduledJob *p_job,
                                  uint8_t spindle_preference)
        {
            uint8_t selected_spindle_index = next_spindle_index;
            /* The next spindle selection could be better,
               by putting onto a thread to run in parallel
               with things that aren't dependent - i.e.
               if no dependencies, put onto a seperate
               thread.*/
            if (spindle_preference != SPINDLE_INVALID)
            {
                selected_spindle_index = spindle_preference;
            }
            else
            {
                next_spindle_index++;
                next_spindle_index =
                    next_spindle_index % spindles.size(); // round robin.
            }

            // spindle ids are indexes.
            spindles[selected_spindle_index].AddJob(p_job);
            return selected_spindle_index;
        }

      public:
        void Describe()
        {
            for (auto &spindle : spindles)
            {
                MYR_INFO("Spindle: {0}", spindle.GetID());
                for (auto &p_job : spindle.GetJobList())
                {
                    MYR_INFO("\tJOB: {0}", p_job->GetName());
                }
            }
        }
        ThreadPoolJobSystem(size_t num_threads)
        {
            MYR_CORE_INFO("ThreadPoolJobSystem creating.");
            if (num_threads == 0)
                num_threads = std::thread::hardware_concurrency();
            // So before we start, reserve enough space in the
            // vector. This is to stop it reallocating memory
            // and destructing objects. Otherwise you get
            // move/copy semantics happening on the spindles
            // when the vector is resized and destructors of
            // old spindles start getting called which we
            // don't want. It also seems inefficient.
            spindles.reserve(num_threads);
            threads.reserve(num_threads);

            for (size_t i = 0; i < num_threads; ++i)
            {
                // Create the spindle with the same id as its position.
                spindles.emplace_back((uint8_t)i);
            }
            for (auto i = 0; i < num_threads; ++i)
            {
                // create the thread using the corresponding
                // Spindle
                threads.emplace_back(&Spindle::SpindleSpinner, &spindles[i]);
            }
        }
        virtual ~ThreadPoolJobSystem()
        {
            MYR_CORE_ERROR("ThreadPoolJobSystem destructor will try shutting "
                           "down threads.");
            for (auto &s : spindles)
            {
                s.Stop();
            }

            // could have a condition var in here
            // that notifies threads that are waiting,
            // i.e. notify_all()

            for (auto &t : threads)
            {
                t.join();
            }
        }

        // returns the spindle id of the spindle that was joined.
        uint8_t SubmitJob(ScheduledJob *p_job,
                          uint8_t spindle_preference = SPINDLE_INVALID)
        {
            auto found = known_jobs_map.find(p_job);
            if (found != known_jobs_map.end())
            {
                MYR_ERROR("Submitting a job which already exists in the pool. "
                          "Ignoring");
                return SPINDLE_INVALID;
            }

            // Check the job doesn't already exist
            auto allocated_spindle_id =
                AllocateToSpindle(p_job, spindle_preference);
            // If it was actually allocated, then this is now
            // a known job.
            if (allocated_spindle_id != SPINDLE_INVALID)
                known_jobs_map[p_job] = allocated_spindle_id;
            return allocated_spindle_id;
        }
    };
} // namespace Myriad
#endif
