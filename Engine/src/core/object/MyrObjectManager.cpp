#include "core/object/MyrObjectManager.h"

#include "io/Log.h"

namespace Myriad
{
    MyrObjectManager::~MyrObjectManager()
    {
        // TODO: if this has its own allocator, the allocator
        // would need to delete all the gameobjects -> this
        // would trigger deleting all the components, too.
    }

    bool MyrObjectManager::WhenStartService()
    {
        MYR_CORE_INFO("MyrObjectManager Starting");
        return true;
    }
    bool MyrObjectManager::WhenStopService()
    {
        MYR_CORE_INFO("MyrObjectManager Stopping");
        return true;
    }
    void MyrObjectManager::CreateObject() {}
    void MyrObjectManager::DestroyObject() {}
    void MyrObjectManager::AddChild()
    {
        MYR_CORE_TRACE("Myr Object Manager: Add Child");
    }
    void MyrObjectManager::RemoveChild()
    {
        MYR_CORE_TRACE("Myr Object Manager: Remove Child");
    }
    void MyrObjectManager::ReparentChild() {}
} // namespace Myriad
