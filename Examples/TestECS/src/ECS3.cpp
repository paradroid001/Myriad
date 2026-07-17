#include "ECS3.h"

// #include <cstdint> //uint64+t, uint8_t
#include <unordered_map>
#include <set>
#include <string>
#include <cstring> //memcpy
#include <cassert>
#include <iostream>

namespace ECS3
{
  Archetype::Archetype(archetype_t archetype, ECS *ecs) : archetype_(archetype), ecs_(ecs), used_slots(0)
  {
    for (component_type_id_t i = 0; i < MAX_COMPONENT_TYPES; i++)
    {
      // Test each type, build the component_types set,
      // and allocate the component array if needed.
      archetype_t cmp = 1;
      if ((archetype_ & (cmp << i)) == (cmp << i))
      {
        component_types_.insert(i); // this archetype has this component type
      }
      else
      {
        component_arrays_[i] = nullptr;
      }
    }
    // Pre allocate all of the arrays.
    // data will be an array of n pointers.
    data = new void *[component_types_.size()];
    // Each pointer will be an array of max_entities_ of the component type.
    for (auto ctype : component_types_)
    {
      ComponentMeta *meta = &ecs_->component_types_[ctype];
      data[ctype] = operator new(meta->DataSize() * ecs_->max_entities_);
      // Set the component_arrays_ pointer to the data array.
      component_arrays_[ctype] = data[ctype];
    }
    // entity_data_index will be an array of max_entities_ of entity ids.
    entity_data_index = new entity_id_t[ecs_->max_entities_];
    entity_index_ = new size_t[ecs_->max_entities_];
  }

  bool Archetype::HasComponent(component_type_id_t c)
  {
    // TODO: which is faster - the below, or asking the set if it has c?
    archetype_t cmp = 1 << c;
    return (archetype_ & cmp) == cmp;
  }

  // Let this Archetype know what the 'next' archetype would be
  // if component ctype was added to an entity.
  void Archetype::AdviseNext(component_type_id_t ctype, Archetype *arch)
  {
    // assert(next_.count(ctype) == 0); // should not exist
    assert(arch != nullptr);
    next_[ctype] = arch;
  }

  // Let this Archetype know what the 'prev' archetype would be
  // if the component ctype was removed from an entity.
  void Archetype::AdvisePrev(component_type_id_t ctype, Archetype *arch)
  {
    // assert(prev_.count(ctype) == 0); // should not exist
    assert(arch != nullptr);
    prev_[ctype] = arch;
  }

  // Get the entity's index in the component array for this archetype
  // If it doesn't exist, it is added.
  size_t Archetype::AddEntity(entity_id_t entity)
  {
    if (!entities_.count(entity))
    {
      entities_.insert(entity);
      size_t index = used_slots++; // the next available slot
      entity_data_index[index] = entity;
      entity_index_[entity] = index;
    }
    return entity_index_[entity];
  }
  void Archetype::RemoveEntity(entity_id_t entity)
  {
    if (entities_.count(entity))
    {
      // Remove the entity from the set of entities in this archetype
      entities_.erase(entity);
      // Get the index/dataindex of the entity in the component arrays
      size_t index = entity_index_[entity];
      // size_t data_index = entity_data_index[index];

      // We want to now swap the LAST entity data in for this one.
      // This is a simple approach - in a real implementation, you might want to be more sophisticated.
      size_t last_index = used_slots - 1;
      if (last_index != index)
      {
        // Swap the data
        for (auto component_id : component_types_)
        {
          ComponentMeta meta = ecs_->component_types_[component_id];
          uint8_t *data_ptr = static_cast<uint8_t *>(component_arrays_[component_id]);

          uint8_t *dest = &(data_ptr[index * meta.DataSize()]);
          uint8_t *src = &(data_ptr[last_index * meta.DataSize()]);
          std::memcpy(dest, src, meta.DataSize());
        }
        // We've just swapped the last entity's data into the index
        // of the entity being removed.
        // so update the entity index for what was the last entity
        entity_index_[entity_data_index[last_index]] = index;
        // Update the entity data index for the last entity
        entity_data_index[index] = entity_data_index[last_index];
      }
      // Mark the slot as unused
      used_slots--;
    }
  }

  void *const Archetype::Data(size_t index, component_type_id_t ctype)
  {
    uint8_t *data_ptr = (uint8_t *)component_arrays_[ctype];
    ComponentMeta meta = ecs_->component_types_[ctype];
    return &(data_ptr[index * meta.DataSize()]);
  }

  /*
  void *CreateDataForComponent(component_type_id_t ctype, size_t max_entities)
  {
    ComponentMeta meta = component_meta_[ctype];
    void **components = &component_map_[ctype];
    if (*components == nullptr)
    {
      *components = ::operator new(meta.DataSize() * max_entities);
    }
    return *components;
  }
  */

  // Moves all data, the entity membership, and
  // Returns the new archetype
  archetype_t Archetype::MigrateEntityTo(entity_id_t entity, Archetype *dest_arch)
  {
    // To migrate an entity we:
    // 1. add the entity to the new archetype (gives us an index)
    // 2. move all data in components this archetype has that the other has.
    // 3. remove the entity from this archetype
    // 4. Return the entity's new archetype so it can be updated externally.

    // 1. Add the entity to the new archetype, giving us an index for data.
    size_t new_index = dest_arch->AddEntity(entity);

    // 1. Move data. Assumption: the destination Archetype has been set up.
    for (auto ctype : component_types_)
    {
      if (dest_arch->HasComponent(ctype)) // if the dest arch has this component
      {
        // Copy the component data.
        // Dest is the new index in the new arch
        void *dest = dest_arch->Data(new_index, ctype);
        // Src is the current index in this arch
        void *src = Data(entity_index_[entity], ctype);
        // I am just going to straight up memcpy it.
        std::memcpy(dest, src, ecs_->component_types_[ctype].DataSize());
        // std::cout << "Copied " << meta.DataSize() << " bytes for component " << item.first << " from arch " << archetype_ << " to " << arch->archetype_ << std::endl;
      }
    }
    // 3. Remove from this archetype
    RemoveEntity(entity);
    // 4.
    return dest_arch->archetype_;
  }

  // Migrate entities and date to a 'next' Archetype
  archetype_t Archetype::MigrateEntityToNext(entity_id_t entity, component_type_id_t ctype)
  {
    assert(next_.count(ctype) != 0);
    return MigrateEntityTo(entity, next_[ctype]);
  }
  // Migrate entities and data to a 'prev' Archetype
  archetype_t Archetype::MigrateEntityToPrev(entity_id_t entity, component_type_id_t ctype)
  {
    assert(prev_.count(ctype) != 0);
    return MigrateEntityTo(entity, prev_[ctype]);
  }

}
