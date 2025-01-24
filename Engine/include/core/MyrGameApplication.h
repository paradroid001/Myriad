#ifndef MYRIAD_CORE_GAMEAPPLICATION_H
#define MYRIAD_CORE_GAMEAPPLICATION_H

#include "core/core.h"
#include "core/MyrApplication.h"
#include "core/MyrEntityManager.h"
#include "core/MyrProfiling.h"
#include "core/MyrGameEngine.h"

namespace Myriad
{
  class MYR_API MyrGameApplication : public MyrApplication
  {
  protected:
    MyrProfiler profiler_;
    MyrGameEngine engine_;

  public:
    MyrGameApplication();
    virtual ~MyrGameApplication();
    virtual void Run();

    virtual EngineConfig_t Setup() = 0; //<-- Runs before we init the engine.

    /* INIT */
    virtual void Init(EngineConfig_t config); //<-- We init the engine here

    /*START*/
    virtual void Start(); //<-- Start getting your entities created here.

    /* UPDARE */
    virtual void PreUpdate();
    virtual void Update();
    virtual void PostUpdate();

    /* RENDER */
    virtual void PreRender();
    virtual void Render();
    virtual void PostRender();

    /* SHUTDOWN */
    virtual void PreShutdown();
    virtual void Shutdown();
    virtual void PostShutdown();
  };
}
#endif
