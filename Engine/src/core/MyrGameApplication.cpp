#include "string.h" //strncpy

#include "myriad.h"

#include "asset/MyrAssetManager.h"
#include "gfx/Renderer.h"
#include "gfx/Window.h"
#include "io/Logging.h"

#include "tracy/Tracy.hpp"

namespace Myriad
{
    MyrGameApplication::MyrGameApplication() {}

    MyrGameApplication::~MyrGameApplication()
    {
        MYR_CORE_TRACE("MyrGameApplication destroyed");
    }

    void MyrGameApplication::Run()
    {
        GameEngineConfig c;
        c.window_config.resolution = {800, 600};
        c.window_config.resizable = true;
        c.window_config.borderless = false;
        c.window_config.fullscreen = false;
        c.window_config.vsync = false;
        c.framerate = 100;
        strncpy(c.resource_base_path, "shared/res", 100);
        strncpy(c.window_title, "Default Window Title", 100);

        if (engine_.Init(c)) // Sets an initial config, inits members.
        {
            // opens the window, starts engine running.
            // Calls Start on this object.
            // Overridden MyrGameApplication.Start() has a
            // chance to alter config_ before the engine acts on it.
            engine_.Start(*this);
            while (engine_.IsRunning())
            {
                // Calls all update methods on this object
                // Calls all render methods on this object.
                engine_.Frame(*this);
            }
            engine_.Shutdown(*this);
        }
    }

    bool MyrGameApplication::StartHosted(GameEngineConfig &config,
                                         bool open_window)
    {
        Init(config);
        if (!engine_.Init(config))
        {
            return false;
        }

        engine_.Start(*this, open_window);
        return true;
    }

    void MyrGameApplication::TickHostedFrame()
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

    void MyrGameApplication::RenderHostedFrame()
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

    void MyrGameApplication::StopHosted()
    {
        if (engine_.IsRunning())
        {
            engine_.Shutdown(*this);
        }
    }

    // Override these methods in your own applications
    void MyrGameApplication::Init(GameEngineConfig &config) {}
    void MyrGameApplication::Start() {}

    void MyrGameApplication::PreUpdate() {}
    void MyrGameApplication::Update() {}
    void MyrGameApplication::PostUpdate() {}

    void MyrGameApplication::PreRender() {}

    void MyrGameApplication::Render() {}
    void MyrGameApplication::PostRender() {}

    void MyrGameApplication::PreShutdown() {}
    void MyrGameApplication::Shutdown()
    {
        MYR_CORE_TRACE("MyrGameApplication shutting down");
    }
    void MyrGameApplication::PostShutdown() {}

} // namespace Myriad
