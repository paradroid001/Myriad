#include "myriad.h"
#include "TestGameObject.h"

class ScheduledJobbedTest
{

private:
public:
  void InitRender()
  {
    MYR_INFO("Init render");
  }

  void Render()
  {
    MYR_INFO("Render");
  }

  void Update()
  {
    MYR_INFO("Update");
  }

  void Run()
  {
    Myriad::ScheduledJob renderinitjob("Render Init Job", [&]
                                       { this->InitRender(); });
    Myriad::ScheduledJob renderjob("Render Job", [&]
                                   { this->Render(); });
    Myriad::ScheduledJob updatejob("UpdateJob", [&]
                                   { this->Update(); });
    Myriad::ScheduledJob testjob1("TestJob1", [&]
                                  { MYR_CORE_TRACE("Hello from Test Job 1"); });
    Myriad::ScheduledJob testjob2("TestJob2", [&]
                                  { MYR_CORE_TRACE("Hello from Test Job 2"); });
    Myriad::ScheduledJob testjob3("TestJob3", [&]
                                  { MYR_CORE_TRACE("Hello from Test Job 3"); });

    Myriad::ThreadPoolJobSystem pool(3);
    MYR_INFO("Finished making the job system");

    auto spindleid = pool.SubmitJob(&renderinitjob);
    pool.SubmitJob(&renderjob, spindleid);
    pool.SubmitJob(&updatejob);
    pool.SubmitJob(&testjob1);
    pool.SubmitJob(&testjob2);
    pool.SubmitJob(&testjob3);
    pool.SubmitJob(&testjob1);
    pool.SubmitJob(&testjob2);
    pool.SubmitJob(&testjob3);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    pool.Describe();

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
};
