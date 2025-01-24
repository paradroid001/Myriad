#include "core/MyrGameApplication.h"
#include "core/MyrGameEngine.h"
#include <iostream>

namespace Myriad
{
  MyrGameApplication::MyrGameApplication() {

  };
  MyrGameApplication::~MyrGameApplication()
  {
    std::cout << "Myr Game Application Destructor" << std::endl;
  };

  void MyrGameApplication::Run()
  {
    // run setup, get the config, and init the engine.
    Init(Setup());
    Start();
    while (engine_.IsRunning())
    {
      PreUpdate();
      Update();
      PostUpdate();
      PreRender();
      Render();
      PostRender();
    }
    PreShutdown();
    Shutdown();
    PostShutdown();
  }

  // Inits the game engine
  void MyrGameApplication::Init(EngineConfig_t config)
  {
    engine_.InitEngine(config);
  }
  // After the engine is inited, override this to set up other game params
  void MyrGameApplication::Start() {}

  // Runs before any update
  void MyrGameApplication::PreUpdate() {}
  // Run the update
  void MyrGameApplication::Update() {}
  // Run after every update
  void MyrGameApplication::PostUpdate() {}

  // Runs before any render
  void MyrGameApplication::PreRender() {}
  // Run the render
  void MyrGameApplication::Render() {}
  // Run after every render
  void MyrGameApplication::PostRender() {}

  // Runs before shutdown
  void MyrGameApplication::PreShutdown() {}
  // Shuts down the engine, releases all engine data structures
  void MyrGameApplication::Shutdown() {}
  // Run after shutdown
  void MyrGameApplication::PostShutdown() {}

}
