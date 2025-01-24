#include "core/object/MyrObject.h"
#include "core/object/MyrObjectManager.h"

#include "io/Log.h"
#include <cstdint>

namespace Myriad
{
    MyrObject::MyrObject(MyrObjectManager *p_mgr)
        : hself_(MYRHANDLE_INVALID_INDEX), hparent_(MYRHANDLE_INVALID_INDEX),
          name_("None"), destroyed_(false)
    {
        p_object_manager_ = p_mgr; //&MyrObjectManager::GetInstance();
    }

    MyrObject::MyrObject(const std::string object_name, MyrObjectManager *p_mgr)
        : hself_(MYRHANDLE_INVALID_INDEX), hparent_(MYRHANDLE_INVALID_INDEX),
          name_(object_name), destroyed_(false)
    {
        p_object_manager_ = p_mgr; //&MyrObjectManager::GetInstance();
    } // private constructor - you can't create these.

    void MyrObject::SetParent(MyrHandle_T parent)
    {
        // if you are setting my parent, I need to
        // leave my current parent.
        // p_object_manager_->ReparentChild(hself_, hparent_, parent);
        MYR_CORE_ERROR("Set Parent is not yet implemented in MyrObject");
    }

    MyrHandle_T MyrObject::GetChild(uint32_t index)
    {
        // Return the nth child of myself.
        return p_object_manager_->GetChild(hself_, index);
        /*
          if (index < children.size() && index != Myriad::INVALID_CHILD_INDEX)
          {
              return children.at(index);
          }
          return nullptr;
        */
    }

    uint32_t MyrObject::GetChildIndex(MyrHandle_T child)
    {
        /*
          // Find the child, give back the index.
          std::vector<MyrObject *>::iterator index =
              std::find(children.begin(), children.end(), child);
          if (index == children.end())
          {
              // then it wasn't found
              return Myriad::INVALID_CHILD_INDEX;
          }
          return index - children.begin();
        */
        return Myriad::INVALID_CHILD_INDEX;
    }

    uint32_t MyrObject::GetChildCount() { return children_.size(); }

    bool MyrObject::AddChild(MyrHandle_T new_child)
    {
        /*
          if (GetChildIndex(new_child) != Myriad::INVALID_CHILD_INDEX)
          {
              MYR_CORE_ERROR("Attempt to add child, child already exists.");
              return false;
          }
          p_object_manager_->AddChild();
          children.push_back(new_child);
          return true;
        */
        return true;
    }

    bool MyrObject::RemoveChild(MyrHandle_T child)
    {
        /*
          // First find the child.
          uint32_t index = GetChildIndex(child);
          if (index != Myriad::INVALID_CHILD_INDEX)
          {
              std::vector<MyrObject *>::iterator it = children.begin();
              children.erase(it + index);
              return true;
          }
          return false;
        */
        return false;
    }

    uint32_t MyrObject::GetComponentCount() { return components_.size(); }
    uint32_t MyrObject::GetComponentTypeCount()
    {
        return component_map_.size();
    }
    MyrComponent *MyrObject::GetComponent(uint32_t n)
    {
        /*
          if (n < components.size() && n != Myriad::INVALID_COMPONENT_INDEX)
          {
              return components.at(n);
          }
          return nullptr;
        */
        return nullptr;
    }
    uint32_t MyrObject::GetComponentIndex(MyrComponent *component)
    {
        /*
          // Find the component, give back the index.
          std::vector<MyrComponent *>::iterator index =
              std::find(components.begin(), components.end(), component);
          if (index == components.end())
          {
              // then it wasn't found
              return Myriad::INVALID_COMPONENT_INDEX;
          }
          return index - components.begin();
        */
        return Myriad::INVALID_COMPONENT_INDEX;
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
        components_.push_back(component);
        // Init the component with an owner.
        component->InitComponent(this);

        return true;
    }
    bool MyrObject::RemoveComponent(MyrComponent *component)
    {
        /*
        // First find the child.
        uint32_t index = GetComponentIndex(component);
        if (index != Myriad::INVALID_COMPONENT_INDEX)
        {
            std::vector<MyrComponent *>::iterator it = components.begin();
            components.erase(it + index);
            return true;
        }
        return false;
        */
        return false;
    }

    void MyrObject::Destroy()
    {
        // TODO this needs to be updated to use the manager.
        destroyed_ = true;
    }
} // namespace Myriad
