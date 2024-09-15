#ifndef MYRIAD_CORE_OBJECT_MYROBJECTMANAGER_H
#define MYRIAD_CORE_OBJECT_MYROBJECTMANAGER_H

#include <algorithm> // for ::find
#include <unordered_map>
#include <vector>

#include "core/IService.h"
#include "core/MyrSingleton.h"
#include "core/core.h"
#include "core/memory/Allocator.h"
#include "core/memory/MyrHandle.h"
#include "core/object/GameObject.h"
#include "io/Log.h"

namespace Myriad
{
    class MyrObject; // fwd declare

    static const uint32_t MAX_CHILDREN = 500;
    static const uint32_t INVALID_CHILD_INDEX = MAX_CHILDREN + 1;
    static const uint32_t MAX_COMPONENTS = 10;
    static const uint32_t INVALID_COMPONENT_INDEX = MAX_COMPONENTS + 1;

    struct ObjectNode
    {
      private:
        MyrHandle_T hobject_;
        ObjectNode *pparent_;
        std::vector<ObjectNode *> children_nodes_;

      public:
        ObjectNode() : hobject_(MYRHANDLE_INVALID_INDEX), pparent_(nullptr) {}
        ObjectNode(MyrHandle_T object, ObjectNode *parent)
            : hobject_(object), pparent_(parent)
        {
        }
        inline MyrHandle_T GetHObject() const { return hobject_; }
        inline ObjectNode *GetPParent() const { return pparent_; }
        inline void SetPParent(ObjectNode *p) { pparent_ = p; }
        inline uint32_t GetNumChildren() const
        {
            return children_nodes_.size();
        }
        ObjectNode *ChildAt(uint32_t index) const
        {
            if (index > children_nodes_.size())
                return nullptr;
            return children_nodes_[index];
        }
        uint32_t ChildIndex(ObjectNode *child) const
        {
            std::vector<ObjectNode *>::const_iterator search = std::find(
                children_nodes_.begin(), children_nodes_.end(), child);
            if (search != children_nodes_.end())
            {
                // return the distance from the beginning;
                return search - children_nodes_.begin();
            }
            return INVALID_CHILD_INDEX;
        }

        uint32_t InsertChild(ObjectNode *child)
        {
            children_nodes_.push_back(child);
            return children_nodes_.size();
        }
        // could/should these be iterators?
        // void walk_depth_first();
        // void walk_breadth_first();
        void Log(std::string leading_str) const
        {
            MYR_CORE_TRACE("{0}[{1}]", leading_str, hobject_);
            if (children_nodes_.size() > 0)
            {
                for (auto const &node : children_nodes_)
                {
                    if (node == nullptr)
                        MYR_CORE_ERROR("Child was null when logging");
                    else
                        node->Log(leading_str + "-");
                }
            }
        }
    };

    class MYR_API MyrObjectManager : public IService,
                                     public MyrSingleton<MyrObjectManager>
    {
        friend MyrObject; // myrobject can call our non-public functiona
        // This using line prevents errors finding the
        // myrsingleton constructor
        using MyrSingleton<MyrObjectManager>::MyrSingleton;

      protected:
        Allocator allocator_; // We need our own allocator.
        std::map<MyrHandle_T, ObjectNode> nodes_;
        // std::unordered_map<MyrHandle_T, std::vector<MyrHandle_T>> heirarchy_;
        bool WhenStartService() override;
        bool WhenStopService() override;
        ObjectNode *GetObject(MyrHandle_T);
        // Move a child from one parent to another.
        void ReparentChild(ObjectNode *oldparent, ObjectNode *newparent,
                           ObjectNode *child);

      public:
        // MyrObjectManager(); I can't declare my own constructor because of
        // singleton
        ~MyrObjectManager();
        template <typename... Args> MyrHandle_T CreateGameObject(Args... args);

        GameObject *GetGameObject(MyrHandle_T);

        // Flag a child to be destroyed.
        // This will remove it from the heirarchy
        // and all children will be destroyed.
        void DestroyGameObject(MyrHandle_T object);
        MyrHandle_T GetParent(MyrHandle_T child);
        uint32_t GetNumChildren(MyrHandle_T parent);
        // Gets the child at an index
        MyrHandle_T GetChild(uint32_t index, MyrHandle_T parent);
        // Gets the child index of a given child
        uint32_t GetChildIndex(MyrHandle_T child);
        // Add a child to a parent
        // Return the new child count
        uint16_t AddChild(MyrHandle_T parent, MyrHandle_T child);
        // Remove a child from a parent.
        // Return the new child count
        uint16_t RemoveChild(MyrHandle_T parent, MyrHandle_T child);

        void DebugLogObjects();
    };

    // Create a game object
    // Set its internal handle
    // return its handle.
    template <typename... Args>
    MyrHandle_T MyrObjectManager::CreateGameObject(Args... args)
    {
        MyrHandle<GameObject> h = allocator_.Alloc<GameObject>(args...);
        GameObject *g = h.Get();
        MyrHandle_T handle = h.Handle();

        nodes_[handle] = ObjectNode(handle, nullptr);
        // We can do this because we are a friend of the class
        g->SetHandle(handle);
        return handle;
    }
} // namespace Myriad
#endif
