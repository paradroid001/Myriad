#include "core/object/MyrObjectManager.h"
#include "core/memory/MyrHandle.h"

#include "io/Log.h"

namespace Myriad
{
    MyrObjectManager::~MyrObjectManager()
    {
        MYR_CORE_WARN("ObjectManager destructing");
        MYR_CORE_WARN("DID THE SERVICE LOG THAT THE START/STOP?");
    }

    bool MyrObjectManager::WhenStartService()
    {
        MYR_CORE_INFO("MyrObjectManager Starting");
        allocator_.Init();
        return true;
    }
    bool MyrObjectManager::WhenStopService()
    {
        MYR_CORE_INFO("MyrObjectManager Stopping");
        // TODO: if this has its own allocator, the allocator
        // would need to delete all the gameobjects -> this
        // would trigger deleting all the components, too.
        allocator_.Shutdown();
        return true;
    }

    void MyrObjectManager::DebugLogObjects()
    {
        MYR_CORE_TRACE("DEBUG printing Objects");
        for (const auto &[k, v] : nodes_)
        {
            // only print the ones with no parent.
            // they are the root nodes and will contain all the kids.
            if (v.GetPParent() == nullptr)
            {
                v.Log("");
            }
        }
    }

    // TODO: why can't I make this const, with a const iterator?
    // It complains that &nodes_[handle] doesn't match the return type.
    ObjectNode *MyrObjectManager::GetObject(MyrHandle_T handle)
    {
        std::map<MyrHandle_T, ObjectNode>::iterator it = nodes_.find(handle);
        if (it != nodes_.end())
        {
            return &it->second;
            // return &nodes_[handle];
        }
        return nullptr;
    }

    GameObject *MyrObjectManager::GetGameObject(MyrHandle_T handle)
    {
        GameObject *g = allocator_.At<GameObject>(handle);
        return g;
    }

    void MyrObjectManager::ReparentChild(ObjectNode *oldparent,
                                         ObjectNode *newparent,
                                         ObjectNode *child)
    {
        MYR_CORE_TRACE("NOTIMPLEMENTED: Myr Object Manager: ReparentChild");
    }

    // TODO this is pretty inefficient: needs TWO lookups
    MyrHandle_T MyrObjectManager::GetParent(MyrHandle_T child)
    {
        ObjectNode *p_node = GetObject(child);
        if (p_node != nullptr)
        {
            ObjectNode *p_parent = p_node->GetPParent();
            if (p_parent != nullptr)
                return p_parent->GetHObject();
        }
        // Either child didn't exist, or didn't have a parent.
        return MYRHANDLE_INVALID_INDEX;
    }

    uint32_t MyrObjectManager::GetNumChildren(MyrHandle_T parent)
    {
        ObjectNode *pparent = GetObject(parent);
        if (pparent != nullptr)
        {
            return pparent->GetNumChildren();
        }
        return INVALID_CHILD_INDEX;
    }

    MyrHandle_T MyrObjectManager::GetChild(uint32_t index, MyrHandle_T parent)
    {
        ObjectNode *p_node = GetObject(parent);
        if (p_node != nullptr)
        {
            ObjectNode *child = p_node->ChildAt(index);
            if (child != nullptr)
                return child->GetHObject();
        }
        return MYRHANDLE_INVALID_INDEX;

        /*
        ObjectNode node = nodes_[parent];
        // If the parent has any children, it will be a key in the heirarchy_
        // map
        std::unordered_map<MyrHandle_T, std::vector<MyrHandle_T>>::iterator
            search = heirarchy_.find(parent);
        if (search != heirarchy_.end())
        {
            // So this parent has children. See how many.
            std::vector<MyrHandle_T> children = search->second;
            // index should be between 0-(size-1)
            if (index < children.size())
            {
                return children[index];
            }
        }
        return MYRHANDLE_INVALID_INDEX;
        */
    }

    uint32_t MyrObjectManager::GetChildIndex(MyrHandle_T child)
    {
        // 1. Find the child.
        ObjectNode *p_child = GetObject(child);
        if (p_child != nullptr && p_child->GetPParent() != nullptr)
        {
            return p_child->GetPParent()->ChildIndex(p_child);
        }
        return INVALID_CHILD_INDEX;
        /*
        std::unordered_map<MyrHandle_T, std::vector<MyrHandle_T>>::iterator
            search = heirarchy_.find(parent);
        if (search != heirarchy_.end())
        {
            // So this parent has children
            std::vector<MyrHandle_T> children = search->second;
            std::vector<MyrHandle_T>::iterator childsearch =
                std::find(children.begin(), children.end(), child);
            if (childsearch != children.end())
            {
                return childsearch - children.begin();
            }
        }
        return INVALID_CHILD_INDEX;
        */
    }

    // Destroy a game object
    void MyrObjectManager::DestroyGameObject(MyrHandle_T object)
    {
        // 1. Mark the object and all its chain of children destroyed,
        // internally.
        // 2. Remove those objects from the object_ and heirarchy_ collections.
        // 3. Ask the allocator to destroy those objects.
        // TODO
        MYR_CORE_TRACE("NOTIMPLEMENTED: Myr Object Manager: DestroyObject");
    }
    uint16_t MyrObjectManager::AddChild(MyrHandle_T parent, MyrHandle_T child)
    {
        // 1. Get the child
        ObjectNode *p_child = GetObject(child);
        if (p_child == nullptr)
        {
            MYR_CORE_ERROR("AddChild: child did not exist");
            return INVALID_CHILD_INDEX;
        }
        // 2. Get the parent you are trying to add to.
        ObjectNode *p_parent = GetObject(parent);
        if (p_parent == nullptr)
        {
            MYR_CORE_ERROR("AddChild: parent did not exist");
            return INVALID_CHILD_INDEX;
        }

        // If the child does not have a parent, add it.
        ObjectNode *p_old_parent = p_child->GetPParent();
        if (p_old_parent == nullptr)
        {
            p_child->SetPParent(p_parent);
            // if the parent doesn't already have the child (it shouldn't)
            if (p_parent->ChildIndex(p_child) == INVALID_CHILD_INDEX)
            {
                p_parent->InsertChild(p_child);
            }
            else
            {
                MYR_CORE_WARN("AddChild: child had no parent, but parent "
                              "already had child...");
            }
        }
        else
        {
            ReparentChild(p_old_parent, p_parent, p_child);
        }
        return p_parent->GetNumChildren();
    }

    uint16_t MyrObjectManager::RemoveChild(MyrHandle_T child,
                                           MyrHandle_T parent)
    {
        MYR_CORE_TRACE("NOTIMPLEMENTED: Myr Object Manager: Remove Child");
        return 0;
    }
} // namespace Myriad
