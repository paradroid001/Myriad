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
  Myriad::MyrHandle<Myriad::Renderer> renderer_;
  Myriad::MyrHandle<Myriad::Window> window_;
  Myriad::MyrAppPreferences prefs_;

public:
  RenderInitJob(const char *name) : Myriad::Job(name) {}

  void Init(Myriad::MyrAppPreferences &prefs, Myriad::MyrHandle<Myriad::Window> window, Myriad::MyrHandle<Myriad::Renderer> renderer)
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
  Myriad::MyrHandle<Myriad::Renderer> renderer_;
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
      /* The big green circle*/
      renderer_->DrawCircle({100, 100}, 100.0f, {0, 255, 0, 255});
    }

    renderer_->EndDrawing();
  }
};

class UpdateInitJob : public Myriad::Job
{
private:
  std::vector<TestGameObject *> *p_objects;
  Myriad::Allocator *p_allocator;
  int num_objects;

public:
  UpdateInitJob() : Myriad::Job("Update Init") {}

  void Init(Myriad::Allocator &allocator, std::vector<TestGameObject *> &objects, int num_objects)
  {
    p_objects = &objects;
    p_allocator = &allocator;
    this->num_objects = num_objects;
  }

  void Execute()
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
      Myriad::MyrHandle<TestGameObject> h = p_allocator->Alloc<TestGameObject>(p_allocator);
      TestGameObject *p_tgo = h.Get();
      // TestGameObject *p_tgo = &*(allocator.Alloc<TestGameObject>());
      p_objects->push_back(p_tgo);
      int x = distribute_x(generator);
      int y = distribute_y(generator);
      MYR_TRACE("Set position {0}, {1}", x, y);
      p_tgo->SetPosition(x, y);
      MYR_TRACE("Set velocity");
      p_tgo->SetVelocity(50, 50);
    }
  }
};

class UpdateJob : public Myriad::Job
{
private:
  std::vector<TestGameObject *> *object_ps_;

public:
  UpdateJob(const char *name) : Myriad::Job(name)
  {
  }

  void Init(std::vector<TestGameObject *> &objects)
  {
    object_ps_ = &objects;
  }

  /*
  std::vector<TestGameObject *> *Init(Myriad::Allocator &allocator, int num_objects)
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
      Myriad::MyrHandle<TestGameObject> h = allocator.Alloc<TestGameObject>(&allocator);
      TestGameObject *p_tgo = h.Get();
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
  */

protected:
  virtual void Execute() override
  {
    double dt = 1.0f / 60;
    // MYR_WARN("Update");
    for (auto p_obj : *object_ps_)
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
  Myriad::MyrHandle<TestGameObject> objects_; //<-- I don't know what this is?
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

  void Init(Myriad::Allocator *allocator)
  {
    pool_ = allocator->Alloc<Myriad::ThreadPool>(1); // single threaded
    window_ = allocator->Alloc<Myriad::Window>(allocator);
    renderer_ = allocator->Alloc<Myriad::Renderer>();
  }

  void Run(Myriad::Allocator *allocator, Myriad::MyrAppPreferences &prefs)
  {
    Myriad::MyrHandle<GetInputJob> inputjob = allocator->Alloc<GetInputJob>("GetInput Job");
    Myriad::MyrHandle<RenderInitJob> renderinitjob = allocator->Alloc<RenderInitJob>("Render Init Job");
    Myriad::MyrHandle<RenderJob> renderjob = allocator->Alloc<RenderJob>("Render Job");
    Myriad::MyrHandle<UpdateInitJob> updateinitjob = allocator->Alloc<UpdateInitJob>();
    Myriad::MyrHandle<UpdateJob> updatejob = allocator->Alloc<UpdateJob>("Update Job");

    // Allow the live app data to know the dimensions of the created window.
    Myriad::MyrAppData::Instance()->UpdateScreenDimensions(prefs.screen_dimensions);

    // MYR_INFO("Init Render Init Job");
    renderinitjob->Init(prefs, window_, renderer_);

    // Our vector of game objects
    std::vector<TestGameObject *> test_game_objects;

    // This needs to be inited before render. But render depends on it :()
    // MYR_INFO("Init Update Job");
    updateinitjob->Init(*allocator, test_game_objects, 500);
    // Now we give that filled out ref to UpdateJob
    updatejob->Init(test_game_objects); // returns obj[]

    // Now the renderer wants access to those objects too (but as a ptr)
    renderjob->Init(renderer_, &test_game_objects);

    auto game_over = false;
    auto total_frames = 0;
    auto frames = 0;
    while (!game_over)
    {
      if (total_frames == 0)
      {
        // if this is the first run, have to init.
        pool_->Submit(&*renderinitjob);
        // The update init job MUST be done after renderer (window) has inited.
        // Otherwise there is no GL context
        pool_->Submit(&*updateinitjob);
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

    // Before we kill the GL context,
    // Let's get rid of all the TestGameObjects, and dealloc their
    // textures.
    // ACTUALLY SHOULDNT BE DELETED BY THE ALLOCATOR WHEN IT KILLS THE HANDLES?
    // for (auto p_testgameobject : test_game_objects)
    //{
    //  delete p_testgameobject;
    //}

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
