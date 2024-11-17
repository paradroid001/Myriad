#ifndef MYRIAD_CORE_OBJECT_MYROBJECT
#define MYRIAD_CORE_OBJECT_MYROBJECT

#include <cassert>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/core.h"
#include "core/memory/AllocatorProvider.h"
// #include "core/memory/MyrHandle.h"
#include "core/IEnableable.h"
#include "core/component/MyrComponent.h"
#include "core/memory/MyrHandle.h"
#include "io/Log.h"

// #include "static_type_info.h"

namespace Myriad
{
    class MyrObjectManager; // fwd

    // A base class for objects.
    class MYR_API MyrObject
    {
        friend MyrObjectManager; // myrobjectmanager can call our non public
                                 // function

      private:
        // Set our handle after allocation
        // This should only be called by the manager class
        // The handle should be MYRHANDLE_INVALID_HANDLE beforehand.
        inline void SetHandle(MyrHandle_T handle)
        {
            assert(hself_ == MYRHANDLE_INVALID_INDEX);
            hself_ = handle;
        }
        inline void SetParentHandle(MyrHandle_T handle) { hparent_ = handle; }

      protected:
        MyrHandle_T hself_;
        MyrHandle_T hparent_;
        std::string name_;
        bool destroyed_;
        std::vector<MyrHandle_T> children_;
        std::vector<MyrComponent *> components_;
        std::unordered_map<uint16_t, MyrComponent *> component_map_;
        MyrObjectManager *p_object_manager_;

      public:
        // TODO: These constructors could be private, so that
        // the only way to instantiate these is with an allocator
        MyrObject(MyrObjectManager *p_mgr);
        MyrObject(const std::string object_name, MyrObjectManager *p_mgr);

        virtual ~MyrObject() { MYR_CORE_TRACE("Destructing a MyrObject."); }

        const std::string &GetName() const { return name_; }
        bool IsDestroyed() const { return destroyed_; }
        size_t NumChildren() const { return children_.size(); }
        virtual void Destroy();
        // Children
        virtual MyrHandle_T GetChild(uint32_t n);
        virtual uint32_t GetChildIndex(MyrHandle_T child);
        virtual uint32_t GetChildCount();
        virtual bool AddChild(MyrHandle_T new_child);
        virtual bool RemoveChild(MyrHandle_T child);
        virtual void SetParent(MyrHandle_T parent);
        // Components
        virtual uint32_t GetComponentCount();
        virtual uint32_t GetComponentTypeCount();

        // Construct and add a component
        template <typename T, typename... Args> T *AddComponent(Args... args)
        {
            // TODO this is totally wrong...
            T *component = new T(args...);
            if (AddComponentInternal(component))
            {
                component_map_[component->GetType()] = component;
                return component;
            }
            return nullptr;
        }
        template <typename T> T *GetComponent()
        {
            std::unordered_map<uint16_t, MyrComponent *>::iterator search =
                component_map_.find(T::type);
            if (search == component_map_.end())
                return nullptr;
            return static_cast<T *>(search->second);
        }
        bool AddComponentInternal(MyrComponent *component);
        // virtual bool AddComponent(MyrComponent *component);

        virtual bool RemoveComponent(MyrComponent *component);
        virtual MyrComponent *GetComponent(uint32_t n);
        virtual uint32_t GetComponentIndex(MyrComponent *component);
    };

} // namespace Myriad
#endif
