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

        // Start the total timer to track the overall elapsed time
        total_timer_.Start();

        return true;
    }

    void GameEngine::Frame()
    {
        // MYR_CORE_TRACE("GameEngine frame.");

        frame_timer_.Start();
        update_timer_.Start();

        app_.PreUpdate();
        update_delta_timer_.Stop();
        app_.Update(update_delta_timer_.Time());
        update_delta_timer_.Start();
        app_.PostUpdate();

        update_timer_.Stop();
        render_timer_.Start();

        app_.PreRender();
        render_delta_timer_.Stop();
        app_.Render(render_delta_timer_.Time());
        render_delta_timer_.Start();
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
