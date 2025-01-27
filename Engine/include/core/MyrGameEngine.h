#ifndef MYRIAD_CORE_GAMEENGINE_H
#define MYRIAD_CORE_GAMEENGINE_H

#include "core/core.h"
#include <cstdint>

#include "gfx/Renderer.h"
#include "gfx/Window.h"
#include "asset/AssetManager.h"
#include "core/MyrEvent.h"
#include "io/KeyboardInput.h"
#include "util/MyrRandom.h"
#include "game/GameObjectManager.h"
#include "game/GameObject.h"
#include "core/MyrEntityManager.h" //not yet used...

namespace Myriad
{
  typedef struct EngineConfig_t
  {
    Vector2 screen_dimensions;
    const char *window_title;
    int fps;
    const char *asset_root_path;
    bool threads_enabled;
    uint8_t num_threads;

  } EngineConfig_t;

  class MYR_API MyrGameEngine
  {
  protected:
    EngineConfig_t engine_config_;
    Renderer renderer_{this};
    Window window_;
    // GameObjectManager<GameObject> object_manager_;
    //  MyrEntityManager entity_manager_;
    AssetManager asset_manager_;
    MyrEventService event_service_;
    KeyboardInput keyboard_;
    MyrRandom random_;
    static MyrGameEngine *s_Engine;

  public:
    MyrGameEngine();
    virtual ~MyrGameEngine();
    static MyrGameEngine *Engine()
    {
      return s_Engine;
    }
    bool IsRunning();
    bool InitEngine(EngineConfig_t config);
    bool Shutdown();

    inline Renderer &GetRenderer() { return renderer_; }
    inline Window &GetWindow() { return window_; }
    inline AssetManager &GetAssetManager() { return asset_manager_; }
    inline MyrEventService &GetEventService() { return event_service_; }
    inline KeyboardInput &Input() { return keyboard_; }
    // inline GameObjectManager<GameObject> &GetObjectManager() { return object_manager_; }
    // inline MyrEntityManager &GetEntityManager() { return entity_manager_; }
  };
}

#endif
