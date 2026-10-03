#include "io/Logging.h" //for internal logging
#include "myriad.h"

namespace Myriad
{
    GameEngine::GameEngine(GameApplication &app) : app_(app)
    {
        MYR_CORE_TRACE("GameEngine created.");
    }
    GameEngine::~GameEngine() { MYR_CORE_TRACE("GameEngine destroyed."); }

    bool GameEngine::Init(EngineConfig config)
    {
        config_ = config;
        MYR_CORE_TRACE("GameEngine initialized.");
        state_ = EngineState_t::RUNNING;

        update_timer_.Reset();
        render_timer_.Reset();
        frame_timer_.Reset();
        total_timer_.Reset();

        // Start the total timer to track the overall elapsed time
        total_timer_.Start();

        return true;
    }

    void GameEngine::Frame()
    {
        MYR_CORE_TRACE("GameEngine frame.");

        frame_timer_.Start();
        update_timer_.Start();

        app_.PreUpdate();
        app_.Update();
        app_.PostUpdate();

        update_timer_.Stop();
        render_timer_.Start();

        app_.PreRender();
        app_.Render();
        app_.PostRender();

        render_timer_.Stop();
        frame_timer_.Stop();
    }

    void GameEngine::QueueShutdown()
    {
        if (state_ != EngineState_t::SHUTDOWN)
        {
            state_ = EngineState_t::SHUTDOWN;
        }
    }

    void GameEngine::Shutdown()
    {
        MYR_CORE_TRACE("GameEngine shutdown.");
        total_timer_.Stop();
        state_ = EngineState_t::SHUTDOWN;
        app_.PreShutdown();
        app_.Shutdown();
        app_.PostShutdown();
    }
} // namespace Myriad
