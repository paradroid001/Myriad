#ifndef TEST_TESTTHREADPOOL_H
#define TEST_TESTTHREADPOOL_H

#include "myriad.h"

class TestJob : public Myriad::Job
{
public:
  TestJob(const char *name) : Job(name){};

protected:
  virtual void Execute() override
  {
    MYR_WARN("Hello world from a thread job: {0}", name);
    std::this_thread::sleep_for(
        std::chrono::milliseconds(2000));
    MYR_WARN("Goodbye world from a thread job: {0}", name);
  }
};

class TestThreadPool
{
public:
  void Run(Myriad::Allocator &allocator)
  {
    MYR_CORE_INFO("Testing threadpool");
    Myriad::ThreadPool pool(1);
    Myriad::MyrHandle<TestJob> emily = allocator.Alloc<TestJob>("Emily");
    Myriad::MyrHandle<TestJob> alex = allocator.Alloc<TestJob>("Alex");
    Myriad::MyrHandle<TestJob> stephanie = allocator.Alloc<TestJob>("Stephanie");
    Myriad::MyrHandle<TestJob> thomas = allocator.Alloc<TestJob>("Thomas");

    // Myriad::MyrHandle<Job> job_emily = static_cast<Myriad::MyrHandle<Myriad::Job>>(emily);

    // auto x = 42;
    // auto y = int{43};

    // pool.Init();

    // Three iterations of the loop
    for (int i = 0; i < 3; i++)
    {
      pool.Submit(&*emily);
      pool.Submit(&*alex);
      pool.Submit(&*stephanie);
      pool.Submit(&*thomas);
      // pool.Drain();

      // Prevent tasks from being destroyed before the threads are finished.
      MYR_INFO("Main thread sleeping");
      while (pool.IsBusy())
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    MYR_INFO("END Testing threadpool");
    for (auto &info : pool.GetStats())
    {
      info->Print();
    }
  }
};

#endif
