#ifndef MYRIAD_CORE_GAMEENGINE_H
#define MYRIAD_CORE_GAMEENGINE_H

#include "core/core.h"
#include <cstdint>
#include "gfx/Window.h"
#include "game/GameObjectManager.h"
#include "game/GameObject.h"
#include "core/MyrEntityManager.h"
#include "gfx/Renderer.h"

namespace Myriad
{
  typedef struct EngineConfig_t
  {
    Vector2 screen_dimensions;
    const char *window_title;
    int fps;

    bool threads_enabled;
    uint8_t num_threads;

  } EngineConfig_t;

  class MYR_API MyrGameEngine
  {
  protected:
    EngineConfig_t engine_config_;
    Renderer renderer_;
    Window window_;
    // GameObjectManager<GameObject> object_manager_;
    // MyrEntityManager entity_manager_;

  public:
    MyrGameEngine();
    virtual ~MyrGameEngine();
    bool IsRunning();
    bool InitEngine(EngineConfig_t config);
    bool Shutdown();

    inline Renderer &GetRenderer() { return renderer_; }
    inline Window &GetWindow() { return window_; }
    // inline GameObjectManager<GameObject> &GetObjectManager() { return object_manager_; }
    // inline MyrEntityManager &GetEntityManager() { return entity_manager_; }
  };
}

#endif
