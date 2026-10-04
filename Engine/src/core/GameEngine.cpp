#include "io/Logging.h" //for internal logging
#include "myriad.h"

#include "providers/raylib/RenderProviderRaylib.h"
#include "providers/raylib/WindowProviderRaylib.h"

namespace Myriad
{
    GameEngine::GameEngine(GameApplication &app, std::shared_ptr<IWindow> &win,
                           std::shared_ptr<IRenderer> &rend)
        : app_(app), window_(win), renderer_(rend)
    {
        MYR_CORE_TRACE("GameEngine created.");
    }
    GameEngine::~GameEngine() { MYR_CORE_TRACE("GameEngine destroyed."); }

    bool GameEngine::Init(EngineConfig config)
    {
        config_ = config;
        MYR_CORE_TRACE("GameEngine initialized.");
        state_ = EngineState_t::RUNNING;

        std::unique_ptr<IWindowProvider> wprovider =
            std::make_unique<WindowProviderRaylib>();

        window_ = std::make_shared<Window>(std::move(wprovider));
        window_->Open(config.window_config);

        std::unique_ptr<IRenderProvider> rprovider =
            std::make_unique<RenderProviderRaylib>();

        renderer_ = std::make_shared<Renderer>(std::move(rprovider));

        //  Start the total timer to track the overall elapsed time
        total_timer_.Start();

        return true;
    }

    void GameEngine::Frame()
    {
        // MYR_CORE_TRACE("GameEngine frame.");
        if (window_->GetState() != WindowState_t::READY)
        {

            if (window_->GetState() == WindowState_t::CLOSING)
            {
                QueueShutdown();
            }
            else
                MYR_CORE_ERROR("Window is not ready.");
        }
        else
        {
            frame_timer_.Start();

            /* Update*/
            update_timer_.Start();
            app_.PreUpdate();
            update_delta_timer_.Stop();
            app_.Update(update_delta_timer_.Time());
            update_delta_timer_.Start();
            app_.PostUpdate();
            update_timer_.Stop();

            /* Render*/
            render_timer_.Start();
            renderer_->BeginFrame();
            app_.PreRender();
            render_delta_timer_.Stop();
            app_.Render(render_delta_timer_.Time());
            render_delta_timer_.Start();
            app_.PostRender();
            renderer_->EndFrame();
            render_timer_.Stop();

            frame_timer_.Stop();
        }
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
        window_->Close();
        state_ = EngineState_t::SHUTDOWN;
        app_.PreShutdown();
        app_.Shutdown();
        app_.PostShutdown();
    }
} // namespace Myriad
