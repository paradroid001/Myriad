#pragma once

#include "../../Examples/TestECS/src/TestECS.h"

/**
 * @brief Thin adapter that runs TestECS inside the editor-hosted preview loop.
 */
class HostedPreviewGame : public TestECS
{
public:
  /**
   * @brief Initializes and starts the embedded game engine.
   * @param config Engine configuration used for hosted startup.
   * @return true when engine startup succeeds.
   */
  bool StartHosted(Myriad::GameEngineConfig &config)
  {
    Init(config);
    if (!engine_.Init(config))
    {
      return false;
    }

    engine_.Start(*this, false);
    return true;
  }

  /**
   * @brief Executes one update tick for the hosted preview game.
   */
  void TickFrame()
  {
    if (!IsEngineRunning())
    {
      return;
    }

    engine_.StartUpdateTimer();
    PreUpdate();
    Update();
    PostUpdate();
    engine_.StopUpdateTimer();
  }

  /**
   * @brief Executes one render tick for the hosted preview game.
   */
  void RenderPreview()
  {
    if (!IsEngineRunning())
    {
      return;
    }

    engine_.StartRenderTimer();
    PreRender();
    Render();
    PostRender();
    engine_.StopRenderTimer();
  }

  /**
   * @brief Shuts down the hosted preview engine if it is still running.
   */
  void StopHosted()
  {
    if (engine_.IsRunning())
    {
      engine_.Shutdown(*this);
    }
  }
};
