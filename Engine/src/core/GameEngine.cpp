#include <memory> //make_shared

#include "EngineTypes.h" //GameEngine, GameEngineConfig, MyrGameApplication
#include "UtilTypes.h"   //MyrTimer, MyrRandom

#include "asset/MyrAssetManager.h"
#include "gfx/Renderer.h"
#include "gfx/Window.h"
#include "io/Logging.h"

// If we are profiling:
#include "tracy/Tracy.hpp"

namespace Myriad
{
    GameEngine::GameEngine()
    {
        is_running_ = false;
        is_shutdown_ = false;
        window_opened_ = false;
    }
    GameEngine::~GameEngine() { MYR_CORE_TRACE("Game Engine destroyed"); }

    bool GameEngine::Init(GameEngineConfig config)
    {
        config_ = config;
        assets_ = std::make_shared<MyrAssetManager>();
        // We need the myriad namespace here because we defined
        // methods with the same names as these classes.
        window_ = new Myriad::Window();
        renderer_ = new Myriad::Renderer(assets_);
        return true;
    }

    void GameEngine::Start(MyrGameApplication &GA) { Start(GA, true); }

    void GameEngine::Start(MyrGameApplication &GA, bool open_window)
    {
        // Set this first just in case the user does something
        // weird in Init with screensize.
        screensize_ = config_.window_config.resolution;
        GA.Init(config_); // the user can change config options here.
        // The user may have changed the screensize in Init - change again
        screensize_ = config_.window_config.resolution;

        window_opened_ = false;
        if (open_window)
        {
            window_->Open(config_.window_config, config_.window_title);
            window_->SetFPS(config_.framerate);
            window_opened_ = true;
        }

        GA.Start();

        is_running_ = true;
    }

    void GameEngine::Shutdown(MyrGameApplication &GA)
    {
        is_running_ = false;
        if (!is_shutdown_)
        {
            GA.PreShutdown();
            GA.Shutdown();

            MYR_CORE_TRACE("Ending Asset Manager");
            if (assets_ != nullptr)
            {
                assets_.reset();
            }

            if (window_opened_)
            {
                MYR_CORE_TRACE("Ending Renderer");
                if (renderer_ != nullptr)
                {
                    delete renderer_;
                }

                MYR_CORE_TRACE("Closing Window");
                window_->Close();

                MYR_CORE_TRACE("Ending Window");
                if (window_ != nullptr)
                {
                    delete window_;
                }
            }
            else
            {
                MYR_CORE_TRACE(
                    "Skipping window teardown for hosted preview run");
            }
            is_shutdown_ = true;

            GA.PostShutdown();
        }
    }

    void GameEngine::Frame(MyrGameApplication &GA)
    {
        // We check the windowstate every frame.
        if (window_->GetWindowState() != WindowState_t::CLOSING)
        {
            if (window_->WasResized())
            {
                Vector2 old_size = screensize_;
                screensize_ = window_->GetDimensions();
                MYR_CORE_TRACE("Window resized {%d,%d}->{%d,%d}", old_size.x,
                               old_size.y, screensize_.x, screensize_.y);
            }

            frame_timer_.Start();
            update_timer_.Start();

            ZoneScoped; // Tracy profiling scope for main loop

            // Do update things here.
            GA.PreUpdate();
            GA.Update();
            GA.PostUpdate();

            update_timer_.Stop();
            render_timer_.Start();

            renderer_->BeginFrame();
            // Do Render things here
            GA.PreRender();
            GA.Render();
            GA.PostRender();
            renderer_->EndFrame();

            FrameMark; // Tracy frame mark for the end of the frame

            render_timer_.Stop();
            frame_timer_.Stop();
        }
        else
        {
            is_running_ = false;
        }
    }

} // namespace Myriad
