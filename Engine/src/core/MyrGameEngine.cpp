#include "core/MyrGameEngine.h"
#include "io/MyrLogging.h"
namespace Myriad
{
  MyrGameEngine *MyrGameEngine::s_Engine = nullptr; // defined in translation unit.

  MyrGameEngine::MyrGameEngine()
  {
    MYR_CORE_TRACE("Engine constructor");
    if (s_Engine == nullptr)
      s_Engine = this;
    else
      MYR_CORE_ERROR("FATAL: Engine instance was already set");
  }

  MyrGameEngine::~MyrGameEngine()
  {
    MYR_CORE_TRACE("Engine destructor");
  }

  bool MyrGameEngine::IsRunning()
  {
    if (window_.GetWindowState() != WindowState_t::CLOSING)
    {
      return true;
    }
    return false;
  }

  bool MyrGameEngine::InitEngine(EngineConfig_t config)
  {
    engine_config_ = config;
    // Set the static entity manager variable.
    // entity_manager_.SetEntityManager(&entity_manager_);
    window_.Init(static_cast<int>(config.screen_dimensions.x), static_cast<int>(config.screen_dimensions.y), config.window_title);
    window_.SetFPS(config.fps);

    return true;
  }
  bool MyrGameEngine::Shutdown()
  {
    if (window_.GetWindowState() != WindowState_t::CLOSED)
    {
      window_.Close();
    }
    return true;
  }
}
