#include "core/GameEntity.h"
// #include "MyriadConfig.h"
#include "core/Log.h"

namespace Myriad
{
    GameEntity::GameEntity() { MYR_CORE_TRACE("Game Entity Constructor"); }
    GameEntity::~GameEntity() { MYR_CORE_TRACE("Game Entity Destructor"); }

    // IEnablable
    void GameEntity::Enable() { MYR_CORE_TRACE("Game Entity Enable"); }
    void GameEntity::Disable() { MYR_CORE_TRACE("Game Entity Disable"); }
    void GameEntity::OnEnable() { MYR_CORE_TRACE("Game Entity OnEnable"); }
    void GameEntity::OnDisable() { MYR_CORE_TRACE("Game Entity OnDisable"); }

    // IComponentContainer
    void GameEntity::AddComponent(Myriad::ComponentBase *c)
    {
        MYR_CORE_TRACE("GameEntity::AddComponent is not implemented.");
    }

    void GameEntity::RemoveComponent(Myriad::ComponentBase *c)
    {
        MYR_CORE_TRACE("GameEntity::RemoveComponent is not implemented.");
    }

    ComponentBase *GameEntity::GetComponentByName(std::string &name)
    {
        MYR_CORE_TRACE("GameEntity::GetComponentByName is not implemented.");
        return NULL;
    }

    int GameEntity::GetNumChildren() { return 0; }

    GameObjectBase **GameEntity::GetChildren()
    {
        MYR_CORE_TRACE("GameEntity::GetChildren not implemented");
        return 0;
    }
    GameObjectBase *GameEntity::GetChild(int i)
    {
        MYR_CORE_TRACE("GameEntity::GetChild not implemented");
        return (GameObjectBase *)this;
    }

    int GameEntity::AddChild(GameObjectBase *child)
    {
        MYR_CORE_TRACE("GameEntity::AddChild not implemented");
        return 0;
    }

    int GameEntity::RemoveChild(GameObjectBase *child)
    {
        MYR_CORE_TRACE("GameEntity::RemoveChild not implemented");
        return 0;
    }

    int GameEntity::RemoveChildn(int n)
    {
        MYR_CORE_TRACE("GameEntity::RemoveChildn not implemented");
        return 0;
    }

} // namespace Myriad