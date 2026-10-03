#include "io/Logging.h" //for internal logging
#include "myriad.h"

namespace Myriad
{
    GameApplication::GameApplication() : Application(), engine_(*this)
    {
        MYR_CORE_TRACE("GameApplication created.");
    }
    GameApplication::~GameApplication()
    {
        MYR_CORE_TRACE("GameApplication destroyed.");
    }

    void GameApplication::Run()
    {
        MYR_CORE_TRACE("GameApplication run.");

        EngineConfig config = {
            .window_config = {.resolution = {800, 600},
                              .fullscreen = false,
                              .resizable = true,
                              .vsync = false,
                              .borderless = false},
            .resource_base_path = "shared/res",
            .target_framerate = 60,
        };

        if (engine_.Init(config))
        {
            MYR_CORE_TRACE("GameEngine initialized successfully.");
            Start();
        }
        else
        {
            MYR_CORE_CRITICAL("GameEngine failed to initialize.");
        }

        while (engine_.IsRunning())
        {
            engine_.Frame();
            float time = engine_.GetTotalTimer().Time();
            if (time >= 1000.0f * 10)
            {
                engine_.QueueShutdown();
            }
            else
            {
                MYR_CORE_TRACE("Total elapsed time: %f", time);
            }
        }

        if (engine_.GetState() == EngineState_t::SHUTDOWN)
        {
            engine_.Shutdown();
        }
    }

    void GameApplication::Start() { MYR_CORE_TRACE("GameApplication start."); }

    void GameApplication::PreUpdate()
    {
        // R_CORE_TRACE("GameApplication pre-update.");
    }
    void GameApplication::Update()
    {
        // R_CORE_TRACE("GameApplication update.");
    }
    void GameApplication::PostUpdate()
    {
        // R_CORE_TRACE("GameApplication post-update.");
    }

    void GameApplication::PreRender()
    {
        // R_CORE_TRACE("GameApplication pre-render.");
    }
    void GameApplication::Render()
    {
        // R_CORE_TRACE("GameApplication render.");
    }
    void GameApplication::PostRender()
    {
        // R_CORE_TRACE("GameApplication post-render.");
    }

    void GameApplication::PreShutdown()
    {
        MYR_CORE_TRACE("GameApplication pre-shutdown.");
    }
    void GameApplication::Shutdown()
    {
        MYR_CORE_TRACE("GameApplication shutdown.");
    }
    void GameApplication::PostShutdown()
    {
        MYR_CORE_TRACE("GameApplication post-shutdown.");
    }
} // namespace Myriad
