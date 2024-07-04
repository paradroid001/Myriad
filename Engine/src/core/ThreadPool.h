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
        size_t milliseconds;

        RunJobInfo(const char *name) { jobname = name; };

        void Start() { start = std::chrono::high_resolution_clock::now(); }
        void End() { end = std::chrono::high_resolution_clock::now(); };
        size_t Elapsed()
        {
            milliseconds =
                std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                      start)
                    .count();
            return milliseconds;
        };
    };

    class MYR_API ThreadPool : public IJobSystem
    {
      private:
        // worker threads
        std::vector<std::thread> _threads;
        // queue of tasks
        std::queue<Job *> _jobs;
        // mutex
        std::mutex _queue_mutex;
        // condtion variable signals queue changes
        std::condition_variable _cv;
        // Should the queue stop?
        bool _stop = false;

        std::vector<RunJobInfo> _jobinfo;

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
                                task = std::move(_jobs.front()->GetTask());
                                _jobs.pop();
                            }
                            // here the queue will be unlocked.
                            task(); // run the task
                        }
                    });
            }
        }

        ~ThreadPool() { Drain(); }

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

        void AddJob(Job *job) override
        {
            {
                std::unique_lock<std::mutex>(_queue_mutex);
                _jobs.emplace(std::move(job));
            }
            _cv.notify_one(); // wake up a thread to do some work.
        }
    };
} // namespace Myriad

#endif
