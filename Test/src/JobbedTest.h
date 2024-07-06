#include "myriad.h"
#include "TestGameObject.h"

class RenderJob : public Myriad::Job
{
public:
  RenderJob(const char *name) : Myriad::Job(name){};

protected:
  virtual void Execute() override
  {
    MYR_WARN("Render!");
  }
};

class UpdateJob : public Myriad::Job
{
public:
  UpdateJob(const char *name) : Myriad::Job(name){};

protected:
  virtual void Execute() override
  {
    MYR_WARN("Update");
  }
};

class GetInputJob : public Myriad::Job
{
public:
  GetInputJob(const char *name) : Myriad::Job(name){};

protected:
  virtual void Execute() override
  {
    MYR_WARN("Get Input");
  }
};

class JobbedTestGame
{
private:
  Myriad::MyrHandle<Myriad::Renderer> renderer_;
  Myriad::MyrHandle<TestGameObject> objects_;
  Myriad::MyrHandle<Myriad::ThreadPool> pool_;
  Myriad::MyrHandle<Myriad::Window> window_;

public:
  JobbedTestGame()
  {
  }

  ~JobbedTestGame()
  {
    // allocator was given to us.
  }

  void Init(Myriad::AllocatorService &allocator, Myriad::MyrAppPreferences &prefs)
  {
    renderer_ = allocator.Alloc<Myriad::Renderer>();
    pool_ = allocator.Alloc<Myriad::ThreadPool>(1); // single threaded
    window_ = allocator.Alloc<Myriad::Window>();
    MYR_INFO("Opened Window");
    window_->SetFPS(60);
    window_->Init(prefs.screen_dimensions.x, prefs.screen_dimensions.y, "Jobbed Test Game");
  }

  void Run(Myriad::AllocatorService &allocator)
  {
    Myriad::MyrHandle<GetInputJob> inputjob = allocator.Alloc<GetInputJob>("GetInput Job");
    Myriad::MyrHandle<RenderJob> renderjob = allocator.Alloc<RenderJob>("Render Job");
    Myriad::MyrHandle<UpdateJob> updatejob = allocator.Alloc<UpdateJob>("Update Job");

    while (!window_->ShouldClose())
    {
      pool_->Init(); // reset 'stop' var
      // Add jobs to the queue.
      // these will run in order since it is singlethreaded.
      pool_->AddJob(&*inputjob);
      pool_->AddJob(&*updatejob);
      pool_->AddJob(&*renderjob);

      // run the queue.
      MYR_INFO("Draining");
      pool_->Drain();
      MYR_INFO("Drained");
      std::this_thread::sleep_for(
          std::chrono::milliseconds(15));
    }

    MYR_INFO("Closed Window");

    for (auto &info : pool_->GetStats())
    {
      info->Print();
    }
  }
};
