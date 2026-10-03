#ifndef MYRIAD_H
#define MYRIAD_H

#include "myriad_config.h"
#include "myriad_types.h"
#include <memory> //shared_ptr

namespace Myriad
{

    // Create a global loggers to be used in Log calls.
    // Since these need to be used in macros inserted into
    // Any code, the client one needs MYR_API.
    extern MYR_API std::shared_ptr<ILogger> global_logger_core;
    extern MYR_API std::shared_ptr<ILogger> global_logger_client;

#define MYR_TRACE(...)                                                         \
    Myriad::global_logger_client->Log(                                         \
        Myriad::ILogger::MyrLogLevel_t::MYR_LOGLEVEL_TRACE, __VA_ARGS__)
#define MYR_INFO(...)                                                          \
    Myriad::global_logger_client->Log(                                         \
        Myriad::ILogger::MyrLogLevel_t::MYR_LOGLEVEL_INFO, __VA_ARGS__)
#define MYR_WARN(...)                                                          \
    Myriad::global_logger_client->Log(                                         \
        Myriad::ILogger::MyrLogLevel_t::MYR_LOGLEVEL_WARNING, __VA_ARGS__)
#define MYR_ERROR(...)                                                         \
    Myriad::global_logger_client->Log(                                         \
        Myriad::ILogger::MyrLogLevel_t::MYR_LOGLEVEL_ERROR, __VA_ARGS__)
#define MYR_CRITICAL(...)                                                      \
    Myriad::global_logger_client->Log(                                         \
        Myriad::ILogger::MyrLogLevel_t::MYR_LOGLEVEL_CRITICAL, __VA_ARGS__)

    class MYR_API Application
    {
      public:
        Application();
        virtual ~Application();

        virtual void Run() = 0;
    };
    // The CreateApplication function is declared in myriad_entrypoint.h
    // to be included once by the client application and implemented.

    enum class EngineState_t
    {
        UNINITIALIZED,
        RUNNING,
        SHUTDOWN
    };

    class MYR_API GameApplication; // fwd

    class MYR_API GameEngine
    {
      protected:
        EngineConfig config_;
        EngineState_t state_;

        // How much time does the update portion of the frame take?
        Util::Timer update_timer_;
        // How much time does the render portion of the frame take?
        Util::Timer render_timer_;
        // How much time does the entire frame take?
        Util::Timer frame_timer_;
        // How much time has elapsed since the engine started?
        Util::Timer total_timer_;
        Util::Timer update_delta_timer_; // How much time has elapsed since the
                                         // last update?
        Util::Timer render_delta_timer_; // How much time has elapsed since the
                                         // last render?
        Util::Random rand_;
        GameApplication &app_;

      public:
        // We need to be passed an app so that during the engine's lifecycle,
        // it can communicate with the application and call the
        // pre/update/render/shutdown hooks.
        GameEngine(GameApplication &app);
        virtual ~GameEngine();

        // Initialise the engine
        virtual bool Init(EngineConfig config);
        // Do everything for a single frame
        virtual void Frame();
        virtual void QueueShutdown();
        virtual void Shutdown();

        virtual inline EngineState_t GetState() const { return state_; }
        virtual inline EngineConfig GetConfig() const { return config_; }
        virtual inline Util::Timer GetUpdateTimer() const
        {
            return update_timer_;
        }
        virtual inline Util::Timer GetRenderTimer() const
        {
            return render_timer_;
        }
        virtual inline Util::Timer GetFrameTimer() const
        {
            return frame_timer_;
        }
        virtual inline Util::Timer GetTotalTimer() const
        {
            return total_timer_;
        }
        virtual inline bool IsRunning() const
        {
            return state_ == EngineState_t::RUNNING;
        };
    };

    class MYR_API GameApplication : public Application
    {
      protected:
        GameEngine engine_;

      public:
        GameApplication();
        virtual ~GameApplication();
        virtual void Run() override;
        // Here you can set up anything needed
        // before the first frame
        virtual void Start();

        virtual void PreUpdate();
        virtual void Update(float delta_ms);
        virtual void PostUpdate();

        virtual void PreRender();
        virtual void Render(float delta_ms);
        virtual void PostRender();

        virtual void PreShutdown();
        virtual void Shutdown();
        virtual void PostShutdown();
    };

} // namespace Myriad

#endif // MYRIAD_H
