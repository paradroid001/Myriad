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
    // Myriad::AssetManager *p_asset_manager = new Myriad::AssetManager();
    // Myriad::MyrObjectManager *p_mgr = new Myriad::MyrObjectManager();
    Myriad::AssetManager *p_asset_manager = Myriad::MyrApplication::GetEngine()->GetAssetManager();
    Myriad::MyrObjectManager *p_mgr = Myriad::MyrApplication::GetEngine()->GetObjectManager();
    Myriad::MyrHandle<int> hint = allocator.Alloc<int>();
    MYR_TRACE("alloc renderer");
    // Myriad::MyrHandle<Myriad::Renderer> renderer = allocator.Alloc<Myriad::Renderer>();
    Myriad::MyrHandle<Myriad::Renderer> renderer = Myriad::MyrApplication::GetEngine()->GetRendererHandle();
    renderer->Init();
    // Myriad::MyrHandle<Myriad::Window> win = allocator.Alloc<Myriad::Window>(&allocator);
    Myriad::MyrHandle<Myriad::Window> win = Myriad::MyrApplication::GetEngine()->GetWindowHandle();
    MYR_TRACE("Made a window");
    win->SetFPS(60);
    win->Init(prefs.screen_dimensions.x, prefs.screen_dimensions.y, "Test Window");
    // Allow the live app data to know the dimensions of the created window.
    Myriad::MyrApplication::GetAppData()->UpdateScreenDimensions(prefs.screen_dimensions);

    *hint = 0; // start count at 0
    int num_objects = 4000;
    std::random_device os_seed;
    const u32 seed = os_seed();
    engine generator(seed);
    std::uniform_int_distribution<u32> distribute_x(0, Myriad::MyrApplication::GetAppData()->GetScreenDimensions().x);
    std::uniform_int_distribution<u32> distribute_y(0, Myriad::MyrApplication::GetAppData()->GetScreenDimensions().y);

    Myriad::MyrHandle<TestGameObject> objects[num_objects];
    for (int i = 0; i < num_objects; i++)
    {
      // this handle situation isn't going to work...
      objects[i] = allocator.Alloc<TestGameObject>(&allocator, p_asset_manager, p_mgr);

      // objects[i]->SetPosition(distribute_x(generator), distribute_y(generator));
      // objects[i]->SetVelocity(50, 50);
      objects[i]->GetTransform().SetPosition((float)distribute_x(generator), (float)distribute_y(generator), 0.0f);
      // This code might not work, the vector might be read only.
      // Myriad::Vector2 v = static_cast<TestGameObjectUpdater *>(objects[i]->GetUpdater())->GetVelocity();
      // v.x = 50;
      // v.y = 50;
      static_cast<TestGameObjectUpdater *>(objects[i]->GetUpdater())->SetVelocity(50, 50);
    }

    while (!win->ShouldClose() && (*hint) < 6000)
    {
      ++(*hint);
      // Update
      for (int i = 0; i < num_objects; i++)
      {
        objects[i]->GetUpdater()->Update(1.0f / 60);
        // Myriad::Vector2 v = static_cast<TestGameObjectUpdater *>(objects[i]->GetUpdater())->GetVelocity();
        // MYR_INFO("vel is {0}, {1}", v.x, v.y);
      }
      if (*hint % 60 == 0)
      {
        // MYR_INFO("HINT is {0}", *hint);
      }

      // Render
      renderer->BeginDrawing();
      renderer->ClearBackground({0, 0, 255, 255});

      for (int i = 0; i < num_objects; i++)
      {
        objects[i]->GetDrawer()->Draw(*renderer);
      }
      renderer->EndDrawing();
    }

    // delete &win; // should cause Window destructor to run.
    renderer->Shutdown();
    win->Shutdown();
  }
};

#endif
