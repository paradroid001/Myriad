#include "core/MyrGameEngine.h"
#include "game/oc/Component.h"
#include "game/oc/GameObject.h" //so I can set static vars
#include "io/MyrLogging.h"

namespace Myriad
{
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

        window_.SetFPS(config.fps);
        keyboard_.Init();

        // create the GameObject Manager
        p_game_object_manager_ =
            new GameObjectManager(config.max_gameobject_slots);
        p_component_manager_ = new ComponentManager(config.max_component_slots);

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

        if (window_.GetWindowState() != WindowState_t::CLOSED)
        {
            window_.Close();
        }

        delete p_component_manager_;
        delete p_game_object_manager_;

        return true;
    }
} // namespace Myriad
