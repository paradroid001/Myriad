#include "core/MyrGameEngine.h"
#include "game/oc/Component.h"
#include "game/oc/GameObject.h" //so I can set static vars
#include "io/MyrLogging.h"

namespace Myriad
{
    // Create these 'global' convenience structures.
    InputType Input;
    ScreenType Screen;

    MyrGameEngine *MyrGameEngine::s_Engine =
        nullptr; // defined in translation unit.

    MyrGameEngine::MyrGameEngine()
    {
        MYR_CORE_TRACE("Engine constructor");
        // A few static things to take care of.
        // 1. A static pointer to the engine.
        if (s_Engine == nullptr)
            s_Engine = this;
        else
            MYR_CORE_ERROR("FATAL: Engine instance was already set");
        // 2. Events need to know the pointer to the event service.
        MyrEvent::SetEventService(&event_service_);
    }

    MyrGameEngine::~MyrGameEngine() { MYR_CORE_TRACE("Engine destructor"); }

    bool MyrGameEngine::IsRunning()
    {
        if (window_.GetWindowState() != WindowState_t::CLOSING)
        {
            return true;
        }
        return false;
    }

    bool MyrGameEngine::InitEngine(EngineConfig_t config)
    {
        engine_config_ = config;
        // Set the static entity manager variable.
        // entity_manager_.SetEntityManager(&entity_manager_);
        if (false == window_.Init(static_cast<int>(config.screen_dimensions.x),
                                  static_cast<int>(config.screen_dimensions.y),
                                  config.window_title))
        {
            MYR_CORE_ERROR("Unable to init window");
            return false;
        }
        // Constitutes a 'use' of ::Screen in MyrGameEngine
        Screen.width = config.screen_dimensions.x;
        Screen.height = config.screen_dimensions.y;

        window_.SetFPS(config.fps);
        // Keyboard is the extent of what Engine->Input returns.
        keyboard_.Init();
        // Hook up keyboard to the Input convenience struct.
        Myriad::Input.input_device = &keyboard_;

        // create the GameObject Manager
        p_game_object_manager_ =
            new GameObjectManager(config.max_gameobject_slots);
        p_component_manager_ = new ComponentManager(config.max_component_slots);

        // Create the physics system
        p_physics = new PhysicsSystem();

        // 3. GameObject needs to know what managers to use
        GameObject::SetObjectManager(p_game_object_manager_);
        GameObject::SetComponentManager(p_component_manager_);
        // 4. Component needs to know what managers to use.
        ComponentBase::SetObjectManager(p_game_object_manager_);
        ComponentBase::SetComponentManager(p_component_manager_);
        ComponentBase::SetAssetManager(&asset_manager_);

        return true;
    }

    bool MyrGameEngine::Shutdown()
    {
        keyboard_.Shutdown();

        delete p_physics;
        delete p_component_manager_;
        delete p_game_object_manager_;
        return true;
    }
} // namespace Myriad
