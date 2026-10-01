#ifndef _MYRIAD_ENGINETYPES_H_
#define _MYRIAD_ENGINETYPES_H_

#include "EngineConfig.h"

#include "AssetTypes.h"     //AssetManager
#include "CoreTypes.h"      //windowconfig
#include "InterfaceTypes.h" //IRenderer, IWindow
#include "UtilTypes.h"      //MyrTimer, MyrRandom

#include "s8.h"    //Myriad serialisation header
#include <cstring> //for strncpy
#include <string>  //for all the serialisation.

#include "../src/io/Logging.h"

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
        std::string resource_base_path;
        int framerate;
        std::string window_title;

        static std::string TypeName() { return "GameEngineConfig"; }
        static bool Serialise(const GameEngineConfig &obj, s8::Serialiser &s,
                              s8::Serialiser::NodeID_t id)
        {
            /*
            s.Write<WindowConfig>("window_config", obj.window_config, id);
            s.Write<int>("framerate", obj.framerate, id);
            s.Write<std::string>("resource_base_path", obj.resource_base_path,
                                 id);
            s.Write<std::string>("window_title", obj->window_title, id);
            */
            return (
                WindowConfig::Serialise(&obj.window_config, s,
                                        s.ObjectField(id, "window_config")) &&
                s.Value<int>(s.ScalarField(id, "framerate"), obj.framerate) &&
                s.Value<std::string>(s.ScalarField(id, "resource_base_path"),
                                     std::string(obj.resource_base_path)) &&
                s.Value<std::string>(s.ScalarField(id, "window_title"),
                                     std::string(obj.window_title)));
        }
        static bool Deserialise(GameEngineConfig &obj, s8::Serialiser &d,
                                s8::Serialiser::NodeID_t id)
        {
            /*
            auto res =
                d.Read<WindowConfig>("window_config", obj->window_config);
            if (!res)
                return res;
            res = d.Read<int>("framerate", obj->framerate);
            if (!res)
                return res;
            std::string resource_base_path;
            res = d.Read<std::string>("resource_base_path", resource_base_path);
            std::strncpy(obj->resource_base_path, resource_base_path.c_str(),
                         sizeof(obj->resource_base_path));
            obj->resource_base_path[sizeof(obj->resource_base_path) - 1] = '\0';

            if (!res)
                return res;
            std::string window_title;
            res = d.Read<std::string>("window_title", window_title);
            std::strncpy(obj->window_title, window_title.c_str(),
                         sizeof(obj->window_title));
            obj->window_title[sizeof(obj->window_title) - 1] = '\0';
            return res;
            */
            MYR_CORE_TRACE("Deserialising GameEngineConfig");
            bool result =
                d.ReadObject(d.GetFieldID(id, "window_config"),
                             obj.window_config) &&
                d.ReadScalar(d.GetFieldID(id, "framerate"), obj.framerate) &&
                d.ReadScalar(d.GetFieldID(id, "resource_base_path"),
                             obj.resource_base_path) &&
                d.ReadScalar(d.GetFieldID(id, "window_title"),
                             obj.window_title);
            MYR_CORE_TRACE("Framerate: &d", obj.framerate);
            MYR_CORE_TRACE("Resource path: &s", obj.resource_base_path.c_str());
            MYR_CORE_TRACE("Window title: &s", obj.window_title.c_str());
            MYR_CORE_TRACE("Window config: ..."); // Add appropriate tracing for
                                                  // window_config if needed
            MYR_CORE_TRACE("    resolution: %fx%f",
                           obj.window_config.resolution.x,
                           obj.window_config.resolution.y);
            MYR_CORE_TRACE("    resizable: %d", obj.window_config.resizable);
            MYR_CORE_TRACE("    vsync: %d", obj.window_config.vsync);
            MYR_CORE_TRACE("    fullscreen: %d", obj.window_config.fullscreen);
            MYR_CORE_TRACE("    borderless: %d", obj.window_config.borderless);
            return result;
        }
    };

    class MyrGameApplication; // fwd declare

    class MYR_API GameEngine
    {

        friend class MyrGameApplication;

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
        // These are the references that
        //  the user will use to access the engine and assets.
        //  They have to be initialised in the constructor initializer list,
        //  which means they must be instantiated in the GameEngine
        //  constructor before the MyrGameApplication constructor is called.
        //  See GameEngine::GameEngine() - this used to be done in
        //  GameEngine::Init() but that was too late.
        GameEngine &engine;
        AssetManager &assets;

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
