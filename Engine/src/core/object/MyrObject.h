#ifndef MYRIAD_CORE_OBJECT_MYROBJECT
#define MYRIAD_CORE_OBJECT_MYROBJECT

#include <string>
#include <unordered_map>
#include <vector>

#include "core/core.h"
#include "core/memory/AllocatorProvider.h"
// #include "core/memory/MyrHandle.h"
#include "core/IEnableable.h"
#include "core/component/MyrComponent.h"
#include "core/object/MyrObjectManager.h"
#include "io/Log.h"
#include "static_type_info.h"
#include <cstdint>

namespace Myriad
{
    // TODO find a better home for these
    static const uint32_t MAX_CHILDREN = 500;
    static const uint32_t INVALID_CHILD_INDEX = MAX_CHILDREN + 1;
    static const uint32_t MAX_COMPONENTS = 10;
    static const uint32_t INVALID_COMPONENT_INDEX = MAX_COMPONENTS + 1;
    // A base class for objects.
    class MYR_API MyrObject
    {

      protected:
        std::string name;
        MyrObject *parent;
        std::vector<MyrObject *> children;
        std::vector<MyrComponent *> components;
        std::unordered_map<static_type_info::TypeID, MyrComponent *>
            component_map;
        bool destroyed;
        MyrObjectManager *p_object_manager_;

      public:
        MyrObject() : name("None"), parent(NULL), destroyed(false)
        {
            p_object_manager_ = &MyrObjectManager::GetInstance();
        }
        // This is meant to be private, but the allocator then can't call new.
        MyrObject(const std::string object_name)
            : name(object_name), parent(NULL), destroyed(false)
        { /*Nothing*/
            p_object_manager_ = &MyrObjectManager::GetInstance();
        } // private constructor - you can't create these.

        virtual ~MyrObject() { MYR_CORE_TRACE("Destructing a MyrObject."); }

        const std::string &GetName() const { return name; }
        bool IsDestroyed() const { return destroyed; }
        size_t NumChildren() const { return children.size(); }

        virtual void SetParent(MyrObject *parent);
        virtual bool AddChild(MyrObject *new_child);
        virtual bool RemoveChild(MyrObject *child);
        virtual MyrObject *GetChild(uint32_t n);
        virtual uint32_t GetChildIndex(MyrObject *child);
        virtual uint32_t GetChildCount();
        virtual uint32_t GetComponentCount();
        virtual uint32_t GetComponentTypeCount();
        template <typename T> bool AddComponent(T *component)
        {
            if (AddComponentInternal(component))
            {
                component_map[static_type_info::getTypeID<T>()] = component;
                return true;
            }
            return false;
        }
        template <typename T> T *GetComponent()
        {
            std::unordered_map<static_type_info::TypeID,
                               MyrComponent *>::iterator search =
                component_map.find(static_type_info::getTypeID<T>());
            if (search == component_map.end())
                return nullptr;
            return static_cast<T *>(search->second);
        }
        bool AddComponentInternal(MyrComponent *component);
        // virtual bool AddComponent(MyrComponent *component);

        virtual bool RemoveComponent(MyrComponent *component);
        virtual MyrComponent *GetComponent(uint32_t n);
        virtual uint32_t GetComponentIndex(MyrComponent *component);
        virtual void Destroy();
    };

} // namespace Myriad
#endif
