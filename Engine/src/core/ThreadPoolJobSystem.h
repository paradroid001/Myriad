#ifndef MYRIAD_CORE_THREADPOOLJOBSYSTEM_H
#define MYRIAD_CORE_THREADPOOLJOBSYSTEM_H

#include <list>
#include <map>
#include <queue>
#include <thread>
#include <vector>

#include "core/ScheduledJob.h"

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

    // Creates Spindles (max uint8_t)
    // Jobs can be submitted to a global structure which
    // respects dependencies.
    // The dependency jobs must already be present in the system.
    // The scheduler then allocates to the appropriate Spindle.
    class MYR_API ThreadPoolJobSystem
    {
      protected:
        // A map of jobs to the spindle id they are assigned to.
        std::unordered_map<ScheduledJob *, uint8_t> known_jobs_map;
        std::vector<Spindle> spindles;
        std::uint8_t next_spindle_index = 0;
        // better for this to own them, or
        // it's a world of pain inside Spindle.
        std::vector<std::thread> threads;
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
            // and destructing objects. We don't want to destruct
            // anything until the whole job system is destructed.
            spindles.reserve(num_threads);
            threads.reserve(num_threads);

            for (size_t i = 0; i < num_threads; i++)
            {
                // Create the spindle with the same id as its position.
                spindles.emplace_back((uint8_t)i);
            }
            for (auto i = 0; i < num_threads; i++)
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
