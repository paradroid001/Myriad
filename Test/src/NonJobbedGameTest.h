#ifndef TEST_NONJOBBEDGAMETEST_H
#define TEST_NONJOBBEDGAMETEST_H

#include <cctype>
#include <random>

#include "myriad.h"
#include "TestGameObject.h"

using u32 = uint_least32_t;
using engine = std::mt19937;

class NonJobbedGameTest
{
public:
  void Run(Myriad::Allocator &allocator, Myriad::MyrAppPreferences &prefs)
  {
    // Myriad::MyrHandle<Myriad::IService> *allocator = Myriad::AllocatorService::Instance();

    // Myriad::MyrHandle<Myriad::AllocatorService> allocator_handle = *(static_cast<Myriad::MyrHandle<Myriad::AllocatorService> *>(allocator));
    // Myriad::AllocatorService *allocator_ptr = static_cast<Myriad::AllocatorService *>(allocator->ConstPtr());
    // MYR_TRACE("Finished casting allocator");
    // now allocate stuff with it
    // Myriad::MyrHandle<int> hint = (*(Myriad::MyrHandle<Myriad::AllocatorService> *)allocator)->Alloc<int>();
    Myriad::MyrHandle<int> hint = allocator.Alloc<int>();
    MYR_TRACE("Made it into a cast handle");

    // Myriad::Allocator *allocman = new Myriad::Allocator();
    // Myriad::Handle *mate = allocman->Alloc<int>();

    // Open a window
    // Myriad::Window *win = new Myriad::Window();

    // Myriad::MyrHandle<Myriad::Window> *win = new Myriad::MyrHandle<Myriad::Window>(new Myriad::Window);
    // Myriad::MyrHandle<Myriad::WindowProviderRaylib> *win = new Myriad::MyrHandle<Myriad::WindowProviderRaylib>(new Myriad::WindowProviderRaylib);
    Myriad::MyrHandle<Myriad::Window> win = allocator.Alloc<Myriad::Window>(&allocator);

    MYR_TRACE("Made a window");
    win->SetFPS(60);
    win->Init(prefs.screen_dimensions.x, prefs.screen_dimensions.y, "Test Window");
    // Allow the live app data to know the dimensions of the created window.
    Myriad::MyrAppData::Instance()->UpdateScreenDimensions(prefs.screen_dimensions);

    MYR_TRACE("Setting hint to 0");
    *hint = 0; // start count at 0
    MYR_TRACE("hint is 0");
    // Myriad::MyrHandle<Myriad::GameObject> go = allocator.Alloc<Myriad::GameObject>();
    // go->SetPosition(200, 200);
    // go->SetVelocity(100, 100);
    MYR_TRACE("alloc renderer");
    Myriad::MyrHandle<Myriad::Renderer> renderer = allocator.Alloc<Myriad::Renderer>();
    renderer->Init();

    int num_objects = 1;
    std::random_device os_seed;
    const u32 seed = os_seed();
    engine generator(seed);
    std::uniform_int_distribution<u32> distribute_x(0, Myriad::MyrAppData::Instance()->GetScreenDimensions().x);
    std::uniform_int_distribution<u32> distribute_y(0, Myriad::MyrAppData::Instance()->GetScreenDimensions().y);

    Myriad::AssetManager *p_asset_manager = new Myriad::AssetManager();
    Myriad::MyrObjectManager *p_mgr = new Myriad::MyrObjectManager();
    Myriad::MyrHandle<TestGameObject> objects[num_objects];
    for (int i = 0; i < num_objects; i++)
    {
      // this handle situation isn't going to work...
      objects[i] = allocator.Alloc<TestGameObject>(&allocator, p_asset_manager, p_mgr);

      // objects[i]->SetPosition(distribute_x(generator), distribute_y(generator));
      // objects[i]->SetVelocity(50, 50);
      objects[i]->GetTransform().SetPosition((float)distribute_x(generator), (float)distribute_y(generator), 0.0f);
      // This code might not work, the vector might be read only.
      Myriad::Vector2 v = static_cast<TestGameObjectUpdater *>(objects[i]->GetUpdater())->GetVelocity();
      v.x = 50;
      v.y = 50;
    }

    while (!win->ShouldClose() && (*hint) < 6000)
    {
      // render
      // MYR_WARN("Render");
      // MYR_TRACE("Incrementing hint");
      ++(*hint);
      // MYR_TRACE("Incremented hint");
      for (int i = 0; i < num_objects; i++)
      {
        objects[i]->GetUpdater()->Update(1.0f / 60);
      }

      renderer->BeginDrawing();
      renderer->ClearBackground({0, 0, 255, 255});

      for (int i = 0; i < num_objects; i++)
      {
        objects[i]->GetDrawer()->Draw(*renderer);
      }
      renderer->EndDrawing();
      if (*hint % 1000 == 0)
      {
        MYR_INFO("HINT is {0}", *hint);
      }
    }

    // delete &win; // should cause Window destructor to run.
    renderer->Shutdown();
    delete p_asset_manager;
    win->Shutdown();
  }
};

#endif
