#include "core/MyrGameApplication.h"
#include "core/MyrGameEngine.h"
#include "io/MyrLogging.h"
namespace Myriad
{
    MyrGameApplication::MyrGameApplication() {

    };
    MyrGameApplication::~MyrGameApplication()
    {
        MYR_CORE_INFO("Myr Game Application Destructor");
    };

    void MyrGameApplication::Run()
    {
        // run setup, get the config, and init the engine.
        if (Init(Setup()))
        {
            Start();
            while (engine_.IsRunning())
            {
                frame_timer_.Start();
                update_timer_.Start();

                engine_.GetEventService().ProcessEvents();
                // TODO: this is problematic: event handlers
                //  may have created more events.
                //  Might be better if events just pop themselves
                //  out of the queue once they've been handled,
                //  rather than needing a clear events step.
                engine_.GetEventService().ClearEvents();
                PreUpdate();
                Update();
                PostUpdate();
                update_timer_.Stop();
                render_timer_.Start();
                PreRender();
                Render();
                PostRender();
                render_timer_.Stop();
                frame_timer_.Stop();
            }
        }
        else
        {
            MYR_CORE_ERROR("Critical error, unable to init.");
        }
        PreShutdown();
        Shutdown();
        PostShutdown();
    }

    // Inits the game engine
    bool MyrGameApplication::Init(EngineConfig_t config)
    {
        return engine_.InitEngine(config);
    }
    // After the engine is inited, override this to set up other game params
    void MyrGameApplication::Start() {}

    // Runs before any update
    void MyrGameApplication::PreUpdate() {}
    // Run the update
    void MyrGameApplication::Update() {}
    // Run after every update
    void MyrGameApplication::PostUpdate() {}

    // Runs before any render
    void MyrGameApplication::PreRender() {}
    // Run the render
    void MyrGameApplication::Render() {}
    // Run after every render
    void MyrGameApplication::PostRender() {}

    // Runs before shutdown
    void MyrGameApplication::PreShutdown() {}
    // Shuts down the engine, releases all engine data structures
    void MyrGameApplication::Shutdown()
    {
        MYR_CORE_INFO("GameApplication Shutdown: shutting down engine.");
        engine_.Shutdown();
    }
    // Run after shutdown
    void MyrGameApplication::PostShutdown() {}

} // namespace Myriad
