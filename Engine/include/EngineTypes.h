#ifndef _MYRIAD_ENGINETYPES_H_
#define _MYRIAD_ENGINETYPES_H_

#include "EngineConfig.h"

#include "AssetTypes.h"     //AssetManager
#include "CoreTypes.h"      //windowconfig
#include "InterfaceTypes.h" //IRenderer, IWindow
#include "UtilTypes.h"      //MyrTimer, MyrRandom

namespace Myriad
{
    class MYR_API MyrApplication
    {
      public:
        MyrApplication();
        virtual ~MyrApplication();

        virtual void Run() = 0;
    };

    // Clients of the shared library use this to create their application.
    MyrApplication *CreateApplication();

    class MYR_API GameEngineConfig
    {
      public:
        WindowConfig window_config;
        // threading = bool
        // threads = 0 [auto], 1-n
        char resource_base_path[100];
        int framerate;
        char window_title[100];
    };

    class MyrGameApplication; // fwd declare

    class MYR_API GameEngine
    {
      protected:
        GameEngineConfig config_;

        std::shared_ptr<AssetManager> assets_;
        IWindow *window_;
        IRenderer *renderer_;

        MyrTimer update_timer_;
        MyrTimer render_timer_;
        MyrTimer frame_timer_;

        // This will init the rng.
        MyrRandom random_;

        bool is_running_;    // is the engine running?
        bool is_shutdown_;   // has the engine been shut down?
        bool window_opened_; // did the engine actually open a window?

        Vector2 screensize_;

      public:
        GameEngine();
        ~GameEngine();

        AssetManager &Assets() { return *assets_; }
        IWindow &Window() { return *window_; }
        IRenderer &Renderer() { return *renderer_; }

        bool Init(GameEngineConfig config);
        void Start(MyrGameApplication &GA);
        void Start(MyrGameApplication &GA, bool open_window);
        // do everything the engine needs to do in a frame.
        void Frame(MyrGameApplication &GA);
        void Shutdown(MyrGameApplication &GA);

        void StartUpdateTimer() { update_timer_.Start(); }
        void StopUpdateTimer() { update_timer_.Stop(); }
        void StartRenderTimer() { render_timer_.Start(); }
        void StopRenderTimer() { render_timer_.Stop(); }

        inline bool IsRunning() const { return is_running_; }
        inline Vector2 GetScreenSize() { return screensize_; }

        inline uint32_t GetUpdateTimeMS() { return update_timer_.Time(); }
        inline uint32_t GetRenderTimeMS() { return render_timer_.Time(); }
        inline uint32_t GetFrameTimeMS() { return frame_timer_.Time(); }
        inline uint32_t GetFrameElapsedMS() { return frame_timer_.Elapsed(); }
    };

    class MYR_API MyrGameApplication : public MyrApplication
    {
      protected:
        GameEngine engine_;

      public:
        MyrGameApplication();
        virtual ~MyrGameApplication();
        virtual void Run();

        bool StartHosted(GameEngineConfig &config, bool open_window = false);
        void TickHostedFrame();
        void RenderHostedFrame();
        void StopHosted();

        // Alter engine config here
        virtual void Init(GameEngineConfig &config);
        // Anything you want to do before the first frame.
        virtual void Start();

        bool IsEngineRunning() const { return engine_.IsRunning(); }

        /* UPDATE */
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
} // namespace Myriad

#endif
