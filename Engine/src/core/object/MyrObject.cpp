#include "core/object/MyrObject.h"

#include "io/Log.h"
#include <cstdint>

namespace Myriad
{
    void MyrObject::SetParent(MyrObject *parent)
    {
        // if you are setting my parent, I need to
        // leave my current parent.
    }

    bool MyrObject::AddChild(MyrObject *new_child)
    {
        if (GetChildIndex(new_child) != Myriad::INVALID_CHILD_INDEX)
        {
            MYR_CORE_ERROR("Attempt to add child, child already exists.");
            return false;
        }
        p_object_manager_->AddChild();
        children.push_back(new_child);
        return true;
    }

    uint32_t MyrObject::GetChildIndex(MyrObject *child)
    {
        // Find the child, give back the index.
        std::vector<MyrObject *>::iterator index =
            std::find(children.begin(), children.end(), child);
        if (index == children.end())
        {
            // then it wasn't found
            return Myriad::INVALID_CHILD_INDEX;
        }
        return index - children.begin();
    }

    MyrObject *MyrObject::GetChild(uint32_t index)
    {
        if (index < children.size() && index != Myriad::INVALID_CHILD_INDEX)
        {
            return children.at(index);
        }
        return nullptr;
    }

    bool MyrObject::RemoveChild(MyrObject *child)
    {
        // First find the child.
        uint32_t index = GetChildIndex(child);
        if (index != Myriad::INVALID_CHILD_INDEX)
        {
            std::vector<MyrObject *>::iterator it = children.begin();
            children.erase(it + index);
            return true;
        }
        return false;
    }
    uint32_t MyrObject::GetChildCount() { return children.size(); }
    uint32_t MyrObject::GetComponentCount() { return components.size(); }
    uint32_t MyrObject::GetComponentTypeCount() { return component_map.size(); }
    MyrComponent *MyrObject::GetComponent(uint32_t n)
    {
        if (n < components.size() && n != Myriad::INVALID_COMPONENT_INDEX)
        {
            return components.at(n);
        }
        return nullptr;
    }
    uint32_t MyrObject::GetComponentIndex(MyrComponent *component)
    {
        // Find the component, give back the index.
        std::vector<MyrComponent *>::iterator index =
            std::find(components.begin(), components.end(), component);
        if (index == components.end())
        {
            // then it wasn't found
            return Myriad::INVALID_COMPONENT_INDEX;
        }
        return index - components.begin();
    }

    bool MyrObject::AddComponentInternal(MyrComponent *component)
    {
        if (GetComponentIndex(component) != Myriad::INVALID_COMPONENT_INDEX)
        {
            MYR_CORE_ERROR(
                "Attempt to add component, component already exists.");
            return false;
        }
        // Push onto our component vector before initing,
        // just in case the Init depends on this component
        // already being in the list.
        components.push_back(component);
        // Init the component with an owner.
        component->InitComponent(this);

        return true;
    }
    bool MyrObject::RemoveComponent(MyrComponent *component)
    {
        // First find the child.
        uint32_t index = GetComponentIndex(component);
        if (index != Myriad::INVALID_COMPONENT_INDEX)
        {
            std::vector<MyrComponent *>::iterator it = components.begin();
            components.erase(it + index);
            return true;
        }
        return false;
    }

    void MyrObject::Destroy() { destroyed = true; }
} // namespace Myriad
