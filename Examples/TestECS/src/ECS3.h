#ifndef _ECS3_H_
#define _ECS3_H_

#include <cstdint> //uint64+t, uint8_t
#include <unordered_map>
#include <set>
#include <string>
#include <cstring> //memcpy
#include <cassert>
#include <iostream>

namespace ECS3
{
  using entity_id_t = uint64_t;
  using component_type_id_t = uint8_t;
  using archetype_t = uint64_t;                                // 64 bits for an archetype
  const uint8_t MAX_COMPONENT_TYPES = sizeof(archetype_t) * 8; // 64
  const size_t DEFAULT_MAX_ENTITIES = 1024;

  struct ComponentMeta
  {
  protected:
    component_type_id_t id_;
    std::string name_;
    size_t data_size_;

  public:
    inline component_type_id_t ID() { return id_; }
    inline size_t DataSize() { return data_size_; }
  };

  template <typename T>
  struct Component : public ComponentMeta
  {

  public:
    Component(component_type_id_t newid)
    {
      id_ = newid;
      data_size_ = sizeof(T);
      // std::cout << "Component id is " << (int)id << " and data size is " << data_size << std::endl;
    }
  };

  // I don't know about this implementation of ComponentArray below.
  // Access seems like it will be slow.
  // And I want to be able to copy the data when we migrate.
  // So really I want to just memcpy in there, and then set the meta pointers.

  // Maybe it would be better as an array of pointers to the actual data,
  // And then no [] operators
  // OR an array of indexes, and [] operator which does the extra indirection.
  // OR
  // a combination of the above: every array element is an index and a T payload.
  // You access the array by element id, and then the index part of that
  // element tells you where the real payload is. This might bring more of the
  // array into cache because it's sequential, rather than the above method
  // which is actually two (separately allocated) arrays.
  //
  // +---------+
  // |  INDEX  |
  // +---------+
  // | PAYLOAD |
  // +---------+
  //
  // However, you won't be
  // able to just memcpy sections in, because now you have an index and a payload.

  template <typename T>
  struct ComponentArray
  {
  private:
    ComponentMeta *meta_;
    size_t data_size_;
    size_t used_slots;
    T *data_;

  public:
    ComponentArray(ComponentMeta *meta, size_t data_size) : meta_(meta), data_size_(data_size), used_slots(0)
    {
      // Todo check if datasize is 0
      data_ = new T[data_size_];
    }

    ~ComponentArray()
    {
      delete[] data_;
    }

    // TODO: NOTE: Operator overload may not be the best way to
    // do this - it only supports size_t.

    /*
    T &operator[](size_t entity) // this is your entity id
    {
      // choose what to return.
      // If this entity is not in the list, add it and return the slot
      // for the size of the map.
      if (index_map_.count(entity) != 1)
      {
        index_map_[entity] = map_size_;
        map_size_ += 1;
      }
      return data_[(index_map_[entity])];
    }
    */
    // Can't modify the class inside the const operator.
    /*
    const T &operator[](size_t entity) const
    {
      // choose what to return
      if (index_map_.count(entity) != 1)
      {
        index_map_[entity] = map_size_;
        map_size_ += 1;
      }
      return data_[(index_map_[entity])];
    }
    */
  };

  class ECS; // fwd declare

  struct Archetype
  {
    archetype_t archetype_;
    // ECS pointer: so we can access component_meta, as well as
    // num_entities
    ECS *ecs_;

    /* Component Data lives here*/
    // Which component is in which array is in a mostly sparse array
    void *component_arrays_[MAX_COMPONENT_TYPES];
    // Component data is just n arrays, where n is the number
    // of components in the archetype.
    void **data;

    // Maps of which entity is associated with which index.
    entity_id_t *entity_data_index; // by index, which entity is it
    size_t *entity_index_;          // by entity, which index is it
    size_t used_slots;              // how many slots have been used?

    // std::set is sorted (ascending) by default
    std::set<component_type_id_t> component_types_;
    // The entities in this archetype.
    std::set<entity_id_t> entities_;
    //  store pointers to 'next' archetypes as a map
    // i.e. where you go if you 'add' an archetype
    std::unordered_map<component_type_id_t, Archetype *> next_;
    // and 'prev' artchetypes also
    std::unordered_map<component_type_id_t, Archetype *> prev_;

    Archetype() {}; // this is problematic. I think I need a copy constructor?
    Archetype(const Archetype &other) = default;
    Archetype(archetype_t archetype, ECS *ecs);

    bool HasComponent(component_type_id_t c);
    void AdviseNext(component_type_id_t ctype, Archetype *arch);
    void AdvisePrev(component_type_id_t ctype, Archetype *arch);
    size_t AddEntity(entity_id_t entity);
    void RemoveEntity(entity_id_t entity);
    void *const Data(size_t index, component_type_id_t ctype);
    archetype_t MigrateEntityTo(entity_id_t entity, Archetype *arch);
    archetype_t MigrateEntityToNext(entity_id_t entity, component_type_id_t ctype);
    archetype_t MigrateEntityToPrev(entity_id_t entity, component_type_id_t ctype);
  };

  typedef struct
  {
    size_t allocated_bytes;
    size_t used_bytes;
  } MemStats;

  class ECS
  {
    friend struct Archetype; // we are a friend of ECS, can access all its stuff.

  private:
    size_t max_entities_;
    size_t num_entities_;
    // for assigning the next entity id.
    entity_id_t current_entity_id_ = 0;
    // for assigning the next component type id.
    component_type_id_t current_component_type_id_ = 0;
    uint8_t used_component_types = 0;

    // Array of which components an entity has.
    archetype_t *entity_index_;
    // Map of archetypes.
    std::unordered_map<archetype_t, Archetype> archetype_map_;
    // Array of component type pow^2 to component metadata
    ComponentMeta component_types_[MAX_COMPONENT_TYPES];

    template <class T>
    int GetComponentTypeID()
    {
      // This is a postincrement, so the first s_componentId will (correctly) be zero.
      static int s_componentId = this->current_component_type_id_++;
      // Let's not do powers of 2, do that only for archetypes
      //  return 1 << (s_componentId - 1);
      // Let zero be a valid component id, i.e. a component mask of 2^0 = 0x001
      return s_componentId;
    }

    Archetype &CreateArchetype(archetype_t archetype)
    {
      // Create the archetype: this creates the component arrays
      // and sets them to null.
      archetype_map_[archetype] = Archetype(archetype, this);
      Archetype *retval = &archetype_map_[archetype];
      // next, prev, etc: better to fill these
      // in later, when the first migrations take place.
      return *retval;
    }

  public:
    ECS(size_t capacity) : max_entities_(capacity), num_entities_(0)
    {
      entity_index_ = new archetype_t[max_entities_];
      // This is probably a good start, but you can have
      // far more archetypes than the number of component types.
      archetype_map_.reserve(MAX_COMPONENT_TYPES);
    }
    ECS() : ECS(DEFAULT_MAX_ENTITIES) {}

    size_t Count()
    {
      return num_entities_;
    }

    entity_id_t CreateEntity()
    {
      // entity id of zero is ok (after all it is an index)
      entity_id_t new_eid = current_entity_id_++;
      entity_index_[new_eid] = 0; // empty archetype.
      num_entities_++;
      return new_eid;
    }

    Archetype &GetArchetypeForEntity(entity_id_t entity)
    {
      // see if the archetype exists
      archetype_t archetype = entity_index_[entity];
      return GetArchetype(archetype);
    }

    // TODO: does this need to be public?
    Archetype &GetArchetype(archetype_t archetype)
    {
      if (archetype_map_.count(archetype) == 0)
      {
        return CreateArchetype(archetype);
      }
      return archetype_map_[archetype];
    }

    // Gets or creates a component type.
    // TODO this has to be public so that Query can access it.
    template <typename T>
    component_type_id_t GetComponentType()
    {
      component_type_id_t c = GetComponentTypeID<T>();
      if ((c + 1) > used_component_types) // new component type
      {
        used_component_types += 1;
        component_types_[c] = Component<T>(c);
      }
      return c;
    }

    template <typename T>
    archetype_t AddComponent(entity_id_t entity, T component)
    {
      // Get the componentid for this component.
      component_type_id_t ctype = GetComponentType<T>();
      archetype_t old_arch = entity_index_[entity];
      // Result archetype will be...
      archetype_t cmp = 1;
      archetype_t new_arch = entity_index_[entity] | (cmp << ctype);

      // There are now a few things to do.
      //  0. If the entity has not changed archetypes:
      //       don't do anything (don't allow addition)
      //     else
      //       make sure the entity isn't already in the new archetype
      //       this is basically an error state, can't be in two.
      //  1. Migrate the entity from the old archetype, if any.
      //  2. Add this component data to the new archetype.

      if (new_arch == old_arch)
      {
        std::cout << "Can't add component twice, aborting" << std::endl;
        return new_arch;
      }

      Archetype &new_archetype = GetArchetype(new_arch);
      // Is the entity already in the new archetype?
      if (new_archetype.entities_.find(entity) != new_archetype.entities_.end())
      {
        std::cout << "Critical error, entity " << entity << " from archetype " << old_arch << " is ALREADY in deet archetype " << new_arch << std::endl;
        assert(false);
      }

      if (old_arch != 0) // if not coming from nothing
      {
        // Migration time!
        Archetype &old_archetype = GetArchetype(old_arch);

        // If old_arch doesn't contain us, we are screwed.
        if (old_archetype.entities_.find(entity) == old_archetype.entities_.end())
        {
          std::cout << "Critical error entity, " << entity << " from archetype " << old_arch << " wasn't a member of that archetype " << std::endl;
          assert(false);
        }

        // So, old_arch is a prev for new_arch
        new_archetype.AdvisePrev(ctype, &old_archetype);
        // And new_arch is a next for old_arch
        old_archetype.AdviseNext(ctype, &new_archetype);

        // Migrate entity from Old to New.
        // This returns the new archetype but we don't use it.
        old_archetype.MigrateEntityToNext(entity, ctype);
      }

      // Get the new index for data.
      // If we migrated we will already be in the new
      // archetype, but that's ok.
      size_t new_index = new_archetype.AddEntity(entity);
      *static_cast<T *const>(new_archetype.Data(new_index, ctype)) = component;

      // Update the entity's architecture
      entity_index_[entity] = new_arch;
      return new_arch;
    }

    /*
    template <typename T>
    T *GetComponentsForArchMap(archetype_t archetype, component_type_id_t component_type)
    {
      // does the archetype already exist?
      if (components_.count(archetype) == 0)
      {
        // Initialise with empty map
        // components_[archetype] = {};
        // ComponentArchMap cam = components_[archetype];
        // Create each component type, each bit of the archetype
        for (int i = 0; i < MAX_COMPONENT_TYPES; i++)
        {
          // Need a wide enough cmp to handle the bitshifts.
          // Default is int which breaks (repeats) at <<32 boundary.
          archetype_t cmp = 1;
          if ((archetype & (cmp << i)) == (cmp << i))
          {
            std::cout << "allocing components[" << int(archetype) << "][" << i << "]" << std::endl;
            components_[archetype][i] = new T[max_entities_];
          }
        }
      }
      return static_cast<T *>(components_[archetype][component_type]);
    }
    */
    template <typename T>
    T *const GetComponent(entity_id_t entity)
    {
      // 1. get the archetype for the entity
      // 2. get the component map for the archetype
      // 3. get the component data for the entity.
      component_type_id_t ctype = GetComponentType<T>();
      Archetype &archetype = GetArchetypeForEntity(entity);
      // So this will segfault if:
      //   the entity isn't in the archetype (maybe?)
      //   the archetype doesn't have that component (definintely)
      //   the archetype wasn't set up correctly
      size_t index = archetype.AddEntity(entity);
      return static_cast<T *const>(archetype.Data(index, ctype));
    }
  };

  class Query
  {
  private:
    ECS *ecs_;
    archetype_t arch_;

  public:
    Archetype *archetype_;
    Query(ECS *ecs) : ecs_(ecs), arch_(0), archetype_(nullptr) {}
    Query &Reset()
    {
      arch_ = 0;
      archetype_ = nullptr;
      return *this;
    }
    template <typename T>
    Query &With()
    {
      component_type_id_t t = ecs_->GetComponentType<T>();
      archetype_t cmp = 1;
      arch_ |= (cmp << t);
      archetype_ = &ecs_->GetArchetype(arch_);
      return *this;
    }
    // Execute the query
    // Would be good to return an iterator, ready to go.
    std::set<entity_id_t> &Execute()
    {
      return archetype_->entities_;
    }
  };

  class System
  {
  private:
    Query *query_;

  protected:
    ECS *ecs_;
    std::set<entity_id_t> entities_;

  public:
    System(ECS *ecs, Query *q) : query_(q), ecs_(ecs) {}
    std::set<entity_id_t> Entities() { return query_->Execute(); }
    virtual void Update(float dt) {}
  };
}
#endif
