#ifndef MYRIAD_CORE_THREADPOOL_H
#define MYRIAD_CORE_THREADPOOL_H

#include <condition_variable>
#include <functional>
#include <queue>
#include <thread>
#include <vector>

#include "core/IJobSystem.h"
#include "core/Job.h"
#include "core/core.h"
#include "io/Log.h"

namespace Myriad
{
    class MYR_API RunJobInfo
    {
      public:
        std::string jobname;
        std::chrono::high_resolution_clock::time_point start;
        std::chrono::high_resolution_clock::time_point end;
        uint64_t milliseconds;
        uint64_t nanoseconds;

        RunJobInfo(const char *name)
        {
            jobname = name;
            milliseconds = 0;
            nanoseconds = 0;
        };

        void Start()
        {
            this->start = std::chrono::high_resolution_clock::now();

            // std::chrono::duration<uint64_t> s =
            //     std::chrono::duration_cast<std::chrono::duration<uint64_t>>(
            //         this->start.time_since_epoch());
            //
            // MYR_CORE_TRACE("Starting stats for job {0}: {1}", jobname,
            //                s.count());
        }
        void End()
        {
            this->end = std::chrono::high_resolution_clock::now();
            // std::chrono::duration<uint64_t> s =
            //     std::chrono::duration_cast<std::chrono::duration<uint64_t>>(
            //         this->end.time_since_epoch());
            // MYR_CORE_TRACE("Ending stats for job {0}: {1}", jobname,
            // s.count());
        };
        uint64_t Elapsed()
        {
            milliseconds =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    this->end - this->start)
                    .count();
            nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(
                              this->end - this->start)
                              .count();
            return milliseconds;
        };

        void Print()
        {
            Elapsed();
            // std::chrono::duration<double> seconds =
            //     std::chrono::duration_cast<std::chrono::duration<double>>(
            //         end - start);
            MYR_CORE_TRACE("Job {0} took {1}ms ({2}ns)", jobname, milliseconds,
                           nanoseconds);
        }
    };

    class MYR_API ThreadPool : public IJobSystem
    {
      private:
        // worker threads
        std::vector<std::thread> _threads;
        // queue of tasks
        std::queue<std::reference_wrapper<Job>> _jobs;
        // mutex
        std::mutex _queue_mutex;
        // condtion variable signals queue changes
        std::condition_variable _cv;
        // Should the queue stop?
        bool _stop = false;

        // Job Statistics.
        std::vector<RunJobInfo *> _jobstats;

      public:
        ThreadPool(size_t num_threads = std::thread::hardware_concurrency())
        {
            // create worker threads
            for (size_t i = 0; i < num_threads; ++i)
            {
                _threads.emplace_back(
                    [this]
                    {
                        while (true)
                        {
                            std::function<void()> task;
                            RunJobInfo *jobinfo;
                            // Unlock the queue before executing the queue
                            // so other threads can perform enqueue calls
                            {
                                // first lock the queue
                                std::unique_lock<std::mutex> lock(_queue_mutex);
                                // Wait until there is a task to execute or the
                                // pool is stopped.
                                _cv.wait(lock, [this]
                                         { return !_jobs.empty() || _stop; });
                                // exit the thread in the case the pool
                                // is stopped and there are no tasks
                                if (_stop && _jobs.empty())
                                {
                                    return;
                                }

                                MYR_CORE_TRACE("Ready to run a task.");
                                // Get the next task from the queue.
                                task = std::move(_jobs.front().get().GetTask());
                                jobinfo = new RunJobInfo(
                                    _jobs.front().get().GetName());
                                // Put the jobinfo on the stats list.
                                _jobstats.push_back(jobinfo);

                                _jobs.pop();
                            }
                            // here the queue will be unlocked.
                            jobinfo->Start();
                            task(); // run the task
                            jobinfo->End();
                        }
                    });
            }
        }

        ~ThreadPool()
        {
            Drain();
            // By now all the threads have stopped.
            //  need to free all the jobstats
            for (auto &jobinfo : _jobstats)
            {
                delete jobinfo;
            }
        }

        std::vector<RunJobInfo *> &GetStats() { return _jobstats; }

        void Init() override
        {
            // nothing, possibly this should take 'num threads'
        }

        // drain the pool.
        void Drain() override
        {
            {
                // Lock the queue to update the stop flag safely
                std::unique_lock<std::mutex>(_queue_mutex);
                _stop = true;
            }

            // Notify all threads
            _cv.notify_all();

            // Join all worker threads to ensure they have completed
            // their tasks
            for (auto &t : _threads)
            {
                t.join();
            }
        }

        void AddJob(Job &job) override
        {
            {
                std::unique_lock<std::mutex>(_queue_mutex);
                //using references now, no
                //need for the std::move...
                //_jobs.emplace(std::move(job));
                _jobs.emplace(job);
            }
            _cv.notify_one(); // wake up a thread to do some work.
        }
    };
} // namespace Myriad

#endif
