#ifndef MYRIAD_CORE_THREADPOOL_H
#define MYRIAD_CORE_THREADPOOL_H

#include <condition_variable>
#include <functional>
#include <queue>
#include <thread>
#include <vector>

#include "core/core.h"
#include "core/thread/IJobSystem.h"
#include "core/thread/Job.h"
#include "core/thread/ThreadsafeQueue.h"
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

    /*
        this thread pool structure is slightly wrong.
        the 'threads' that we are joining with Drain
        aren't actually our jobs, they are the lambda that we are
        currently in.
        If we do allow it to end, then we can't easily
        reset it for the next frame.
        So we either need something to signal that each
        thread is done, or we need some kind of overarching
        thread object which encapsulates state.
    */
    class MYR_API ThreadPool : public IJobSystem
    {
      private:
        // worker threads
        std::vector<std::thread> _threads;
        // queue of tasks
        // std::queue<IJob *> _jobs;
        ThreadsafeQueue<IJob *> _jobs;
        // mutex
        std::mutex _queue_mutex;
        // condtion variable signals queue changes
        std::condition_variable _cv;
        // Should the queue stop?
        std::atomic_bool _stop = false;
        size_t total_threads; // the total size of the thread pool.
        size_t busy_threads;  // how many of those have a job?
        // Job Statistics.
        std::vector<RunJobInfo *> _jobstats;

      public:
        ThreadPool(size_t num_threads = std::thread::hardware_concurrency())
        {
            total_threads = num_threads;
            busy_threads = 0;

            // create worker threads
            for (size_t i = 0; i < num_threads; ++i)
            {
                _threads.emplace_back(
                    [this]
                    {
                        while (true)
                        {
                            std::function<void()> task;
                            RunJobInfo *jobinfo = nullptr;
                            // Unlock the queue before executing the queue
                            // so other threads can perform enqueue calls

                            // Actually now I am using the threadsafe queue.
                            // So my queue interactions don't need locking.
                            // But unfortunately stop and the jobstats
                            // access does, so we still have the mutex and
                            // condition var in there.

                            {
                                // first lock the queue
                                // std::unique_lock<std::mutex>
                                // lock(_queue_mutex);

                                // Wait until there is a task to execute or the
                                // pool is stopped.
                                //_cv.wait(lock, [this]
                                //         { return !_jobs.empty() || _stop; });
                                while (_jobs.empty() && !_stop)
                                {
                                    std::this_thread::yield();
                                    // MYR_CORE_TRACE("Jobs count? {0}",
                                    //                _jobs.size());
                                }

                                // exit the thread in the case the pool
                                // is stopped and there are no tasks
                                if (_stop && _jobs.empty())
                                {
                                    return;
                                }
                                else
                                {
                                    // MYR_CORE_TRACE("Ready to run a task.");
                                    //  Get the next task from the queue.
                                    auto front_job = _jobs.try_pop();
                                    if (front_job != nullptr)
                                    {
                                        // task =
                                        // std::move(_jobs.front()->GetTask());
                                        task = (*front_job)->GetTask();
                                        jobinfo = new RunJobInfo(
                                            (*front_job)->GetName());
                                        // Put the jobinfo on the stats list.
                                        _jobstats.push_back(jobinfo);
                                        ++busy_threads;
                                    }
                                    // otherwise queue was empty.
                                }
                            }

                            if (jobinfo != nullptr)
                            {
                                // Actually run the task
                                jobinfo->Start();
                                task(); // run the task
                                jobinfo->End();
                            }

                            {
                                // lock the queue so we can update the
                                // busy threads var
                                std::unique_lock<std::mutex> lock(_queue_mutex);
                                --busy_threads;
                            }
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

        // Wait is just drain.
        void Wait() override { Drain(); }

        bool IsBusy() override
        {
            // with this queue you would be 'not busy'
            // if all the threads were stopped, and
            // the queue was empty.
            // For that, we could increment a var
            // when each thread is active, and decrement
            // it when the thread isn't running a job.
            // so if the queue is empty and that var is 0,
            // you aren't busy.
            {
                // Lock the queue to read some status:
                // std::unique_lock<std::mutex>(_queue_mutex);
                if (_jobs.empty() && busy_threads == 0)
                    return false;
            }
            return true;
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

        void Submit(IJob *job) override
        {
            {
                _jobs.push(job);

                // std::unique_lock<std::mutex>(_queue_mutex);
                //  If using ptr, need to std move (do we?)
                //_jobs.emplace(std::move(job));

                // If using a ref, no need (is there?)
                //_jobs.emplace(job);

                // If all else fails?>
                //  _jobs.push(job);
            }
            _cv.notify_one(); // wake up a thread to do some work.
        }
    };
} // namespace Myriad

#endif
