#ifndef MYRIAD_CORE_MYRENTITYMANAGER_H
#define MYRIAD_CORE_MYRENTITYMANAGER_H

#include "core/core.h"
#include "core/MyrEntity.h"
#include "core/MyrComponent.h"
#include "core/MyrProfiling.h"
#include "io/MyrLogging.h"
#include <unordered_map>

namespace Myriad
{

  struct MyrComponentsDescriptorBase
  {
    virtual ~MyrComponentsDescriptorBase() {}
  };
  template <typename T>
  struct MyrComponentsDescriptor : MyrComponentsDescriptorBase
  {
    uint32_t max_components;
    uint32_t next_component;
    T *array;

    MyrComponentsDescriptor() : MyrComponentsDescriptorBase(), max_components(0), next_component(0), array(nullptr)
    {
    }

    MyrComponentsDescriptor(int num_components) : MyrComponentsDescriptor()
    {
      max_components = num_components;
      next_component = 0;
      array = new T[max_components];
    }
    virtual ~MyrComponentsDescriptor()
    {
      if (array != nullptr)
        delete[] array;
      array = nullptr;
      next_component = 0;
    }
  };

  class MyrEntityManager
  {
  protected:
    static MyrEntityManager *s_this_;
    MyrEntity entities_[MYRIAD_MAX_ENTITIES];
    MYR_ID_t next_entity_id_ = 0;
    std::unordered_map<MyrComponentType, MyrComponentsDescriptorBase *> components_;

  public:
    ~MyrEntityManager();
    // This method must be called before client code
    // starts doing things like creating entities.
    // The game application constructor does this.
    static void SetEntityManager(MyrEntityManager *em);

    /// @brief Create an entity
    /// @param ...args (pass the constructor args for the entity)
    /// @return a pointer to the entity
    /// @todo not threadsafe
    template <typename T, typename... Args>
    static MyrEntity *CreateEntity(Args... args)
    {
      PROFILE_SCOPED
      if (s_this_->next_entity_id_ < MYRIAD_MAX_ENTITIES - 1)
      {
        // No need to new, it's already happened - statically declared.
        // s_this_->entities_[s_this_->current_entity_id_] = new T(args...);
        MyrEntity *ret = &s_this_->entities_[s_this_->next_entity_id_];
        ret->SetID(s_this_->next_entity_id_);
        s_this_->next_entity_id_++;
        return ret;
      }
      else
      {
        MYR_CORE_ERROR("Entities exhausted: {0}", s_this_->next_entity_id_);
      }
      return nullptr;
    }

    static MyrEntity *GetEntity(MYR_ID_t entity_id);

    // Create a component
    // You must pass a derivative of MyrComponent or this will break.
    template <typename T, typename... Args>
    static T *CreateComponent(Args... args)
    {
      PROFILE_SCOPED
      T component(args...); // create a component on the stack?
      MYR_CORE_TRACE("Creating Commponent type {0}", static_cast<uint32_t>(component.GetType()));
      MyrComponentType component_type = static_cast<MyrComponentType>(component.GetType());

      std::unordered_map<MyrComponentType, MyrComponentsDescriptorBase *> &components_ = s_this_->components_;

      if (components_.find(component_type) == components_.end())
      {
        // not found
        MYR_CORE_TRACE("Creating new descriptor for type {0}", (int)component_type);
        MyrComponentsDescriptor<T> *descriptor = new MyrComponentsDescriptor<T>(5);
        components_[component_type] = descriptor;
        MYR_CORE_TRACE("Finished creating new descriptor for type {0}", (int)component_type);
      }
      // Get the descriptor for this component type
      MyrComponentsDescriptor<T> *p_d = static_cast<MyrComponentsDescriptor<T> *>(components_[component_type]);
      p_d->array[p_d->next_component] = T(args...); // initialise the component.
      p_d->next_component++;                        // increment where the next free pointer is.
      MYR_CORE_TRACE("Finished creating Commponent type {0}", static_cast<uint32_t>(component.GetType()));
      return static_cast<T *>(&(p_d->array[p_d->next_component - 1]));
    }

    static bool AddComponent(MYR_ID_t entity_id, MYR_ID_t component_id);
    static bool AddComponent(const MyrEntity entity, MyrComponent &component);
  };
}

#endif
