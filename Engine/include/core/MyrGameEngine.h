#ifndef MYRIAD_CORE_GAMEENGINE_H
#define MYRIAD_CORE_GAMEENGINE_H
#include <cstdint>

#include "core/MyrEvent.h"
#include "core/core.h"

#include "asset/AssetManager.h"
#include "gfx/Renderer.h"
#include "gfx/Window.h"
#include "io/KeyboardInput.h"
// #include "game/oc/GameObject.h"
#include "game/oc/ComponentManager.h"
#include "game/oc/GameObjectManager.h"
#include "game/physics/Physics.h"
#include "util/MyrRandom.h"

namespace Myriad
{
    // This namespace brings in GameObjectManager, GameObject, ComponentManager
    using namespace Myriad::ObjectComponent;
    // This brings in the default Physics model we created
    using namespace Myriad::Physics;

    typedef struct EngineConfig_t
    {
        Vector2 screen_dimensions;
        const char *window_title;
        int fps;
        const char *asset_root_path;
        bool threads_enabled;
        uint8_t num_threads;
        MYR_ID_t max_gameobject_slots;
        MYR_ID_t max_component_slots;

    } EngineConfig_t;

    // This class is to pass around so there's a single
    // point of reference for the main systems.
    typedef struct MYR_API EngineResources_t
    {
        Renderer *p_renderer;
        Window *p_window;
        AssetManager *p_asset_manager;
        GameObjectManager *p_game_object_manager;
        ComponentManager *p_component_manager;
        MyrEventService *p_event_service;
        KeyboardInput p_keyboard_input_;
        MyrRandom p_random;
        PhysicsSystem *p_physics;
    } EngineResources_t;

    class MYR_API MyrGameEngine
    {
      protected:
        EngineConfig_t engine_config_;
        Renderer renderer_{this};
        Window window_;
        AssetManager asset_manager_;
        MyrEventService event_service_;
        KeyboardInput keyboard_;
        MyrRandom random_;
        GameObjectManager *p_game_object_manager_;
        ComponentManager *p_component_manager_;
        PhysicsSystem *p_physics;

        static MyrGameEngine *s_Engine;

      public:
        MyrGameEngine();
        virtual ~MyrGameEngine();
        static MyrGameEngine *Engine() { return s_Engine; }
        bool IsRunning();
        bool InitEngine(EngineConfig_t config);
        bool Shutdown();

        inline Renderer &GetRenderer() { return renderer_; }
        inline Window &GetWindow() { return window_; }
        inline AssetManager &GetAssetManager() { return asset_manager_; }
        inline MyrEventService &GetEventService() { return event_service_; }
        inline KeyboardInput &Input() { return keyboard_; }
        inline GameObjectManager &GetGameObjectManager()
        {
            return *p_game_object_manager_;
        }
        inline ComponentManager &GetComponentManager()
        {
            return *p_component_manager_;
        }
        inline PhysicsSystem &GetPhysicsSystem() { return *p_physics; }

        // inline MyrEntityManager &GetEntityManager() { return entity_manager_;
        // }
    };

    // So these are the convenience
    // Functions that we expect userland
    // to use.

    // inline KeyboardInput &Input()
    //{
    //   return MyrGameEngine::Engine()->Input();
    // }
    // extern KeyboardInput &Input = MyrGameEngine::Engine()->Input();

    // This is used as a convenience:
    // Doing stuff using 'Myriad::Screen.width/2' or whatever
    // The actual instance of this is created
    // in MyrGameEngine.cpp
    struct MYR_API ScreenType
    {
        float width;
        float height;
    };
    extern MYR_API ScreenType Screen;

    // This is used as a convenience:
    //  Myriad::Input::IsKeyDown(xxxx);
    // The actual instance of this is created
    // in MyrGameEngine.cpp
    struct MYR_API InputType
    {
        Myriad::KeyboardInput *input_device;
        inline bool IsKeyDown(KeyCode_t k)
        {
            return input_device->IsKeyDown(k);
        }
    };
    extern MYR_API InputType Input;

    inline MYR_ID_t CreateObject()
    {
        return MyrGameEngine::Engine()
            ->GetGameObjectManager()
            .CreateObject<GameObject>();
    }
    inline GameObject *GetObject(MYR_ID_t id)
    {
        return MyrGameEngine::Engine()->GetGameObjectManager().GetObject(id);
    }

} // namespace Myriad
#endif
