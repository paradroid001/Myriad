#include <vector>
#include <cctype>
#include <random>
// For random numbers
using u32 = uint_least32_t;
using engine = std::mt19937;

#include "myriad.h"
#include "TestGameObject.h"

class RenderInitJob : public Myriad::Job
{
private:
  uint16_t renderer_;
  uint16_t window_;
  uint16_t prefs_;

public:
  RenderInitJob(const char *name) : Myriad::Job(name) {}

  void Init(Myriad::MyrAppPreferences &prefs, uint16_t window, uint16_t renderer)
  {
    prefs_ = prefs;
    renderer_ = renderer;
    window_ = window;
  }

protected:
  virtual void Execute() override
  {
    window_->Init(prefs_.screen_dimensions.x, prefs_.screen_dimensions.y, "Jobbed Game Loop");
    window_->SetFPS(prefs_.target_fps);
    MYR_INFO("Opened Window");
    renderer_->Init();
  }
};

class RenderJob : public Myriad::Job
{
private:
  uint16_t renderer_;
  std::vector<TestGameObject *> *p_object_ps_;

public:
  RenderJob(const char *name) : Myriad::Job(name){};
  void Init(Myriad::MyrHandle<Myriad::Renderer> renderer, std::vector<TestGameObject *> *const p_objects)
  {
    renderer_ = renderer;
    p_object_ps_ = p_objects;
  }

protected:
  virtual void Execute() override
  {
    renderer_->BeginDrawing();
    renderer_->ClearBackground({0, 0, 255, 255});

    for (auto &object_p : *p_object_ps_)
    {
      object_p->Draw(*renderer_);
      renderer_->DrawCircle({100, 100}, 100.0f, {0, 255, 0, 255});
    }

    renderer_->EndDrawing();
  }
};

class UpdateJob : public Myriad::Job
{
private:
  std::vector<TestGameObject *> object_ps_;

public:
  UpdateJob(const char *name) : Myriad::Job(name)
  {
  }
  std::vector<TestGameObject *> *Init(Myriad::AllocatorProvider &allocator, int num_objects)
  {
    std::random_device os_seed;
    const u32 seed = os_seed();
    engine generator(seed);
    std::uniform_int_distribution<u32> distribute_x(0, Myriad::MyrAppData::Instance()->GetScreenDimensions().x);
    std::uniform_int_distribution<u32> distribute_y(0, Myriad::MyrAppData::Instance()->GetScreenDimensions().y);
    MYR_TRACE("Generating Objects");
    for (int i = 0; i < num_objects; i++)
    {
      // this handle situation isn't going to work...
      uint16_t gameobject_id = allocator.Alloc<TestGameObject>();
      TestGameObject *p_tgo = allocator.Get<TestGameObject>(gameobject_id);
      // TestGameObject *p_tgo = &*(allocator.Alloc<TestGameObject>());
      object_ps_.push_back(p_tgo);
      int x = distribute_x(generator);
      int y = distribute_y(generator);
      MYR_TRACE("Set position {0}, {1}", x, y);
      p_tgo->SetPosition(x, y);
      MYR_TRACE("Set velocity");
      p_tgo->SetVelocity(50, 50);
    }

    return &object_ps_;
  };

protected:
  virtual void Execute() override
  {
    double dt = 1.0f / 60;
    // MYR_WARN("Update");
    for (auto p_obj : object_ps_)
    {
      p_obj->Update(dt);
    }
  }
};

class GetInputJob : public Myriad::Job
{
public:
  GetInputJob(const char *name) : Myriad::Job(name){};

protected:
  virtual void Execute() override
  {
    // MYR_WARN("Get Input");
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

  void Init(Myriad::AllocatorProvider *allocator)
  {
    pool_ = allocator->Alloc<Myriad::ThreadPool>(1); // single threaded
    window_ = allocator->Alloc<Myriad::Window>();
    renderer_ = allocator->Alloc<Myriad::Renderer>();
  }

  void Run(Myriad::AllocatorProvider *allocator, Myriad::MyrAppPreferences &prefs)
  {
    Myriad::MyrHandle<GetInputJob> inputjob = allocator->Alloc<GetInputJob>("GetInput Job");
    Myriad::MyrHandle<RenderInitJob> renderinitjob = allocator->Alloc<RenderInitJob>("Render Init Job");
    Myriad::MyrHandle<RenderJob> renderjob = allocator->Alloc<RenderJob>("Render Job");
    Myriad::MyrHandle<UpdateJob> updatejob = allocator->Alloc<UpdateJob>("Update Job");

    // Allow the live app data to know the dimensions of the created window.
    Myriad::MyrAppData::Instance()->UpdateScreenDimensions(prefs.screen_dimensions);

    MYR_INFO("Init Update Job");
    auto p_object_ps = updatejob->Init(*allocator, 500); // returns obj[]

    MYR_INFO("Init Render Init Job");
    renderinitjob->Init(prefs, window_, renderer_);
    renderjob->Init(renderer_, p_object_ps);
    auto game_over = false;
    auto total_frames = 0;
    auto frames = 0;
    while (!game_over)
    {
      if (total_frames == 0)
      {
        // if this is the first run, have to init.
        pool_->Submit(&*renderinitjob);
      }
      // Add jobs to the queue.
      // these will run in order since it is singlethreaded.
      pool_->Submit(&*inputjob);
      pool_->Submit(&*updatejob);
      pool_->Submit(&*renderjob);

      // run the queue.
      while (pool_->IsBusy())
      {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(1));
      }

      if (window_->ShouldClose())
        game_over = true;
      frames += 1;
      if (frames >= 60)
      {
        MYR_INFO("Frames rendered: {0}", frames);
        frames = 0;
      }
      ++total_frames;
    }

    MYR_INFO("Closed Window");
    renderer_->Shutdown();
    window_->Shutdown();

    if (false)
    {
      for (auto &info : pool_->GetStats())
      {
        info->Print();
      }
    }
  }
};
