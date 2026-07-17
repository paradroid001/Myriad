#ifndef _ECS2_H_
#define _ECS2_H_

#include <cstdint> //uint64+t, uint8_t
#include <unordered_map>
#include <set>
#include <string>
#include <queue>
#include <cstring> //memcpy
#include <cassert>

#include <iostream> //cout

namespace ECS2
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
    inline std::string &Name() { return name_; }
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

  struct Archetype
  {
    archetype_t archetype_;
    // std::set is sorted (ascending) by default
    std::set<entity_id_t> entities_;
    // component data.
    std::unordered_map<component_type_id_t, void *> component_map_;
    //  store pointers to 'next' archetypes as a map
    // i.e. where you go if you 'add' an archetype
    std::unordered_map<component_type_id_t, Archetype *> next_;
    // and 'prev' artchetypes also
    std::unordered_map<component_type_id_t, Archetype *> prev_;

    // Component metadata from the ECS. Needed for migrations.
    ComponentMeta *component_meta_;

    Archetype() {}; // this is problematic. I think I need a copy constructor?
    Archetype(const Archetype &other) = default;
    Archetype(archetype_t archetype, ComponentMeta *component_meta) : archetype_(archetype), component_meta_(component_meta)
    {
    }

    bool HasComponent(component_type_id_t c)
    {
      archetype_t cmp = 1 << c;
      return (archetype_ & cmp) == cmp;
    }

    // Let this Archetype know what the 'next' archetype would be
    // if component ctype was added to an entity.
    void AdviseNext(component_type_id_t ctype, Archetype *arch)
    {
      // assert(next_.count(ctype) == 0); // should not exist
      assert(arch != nullptr);
      next_[ctype] = arch;
    }

    // Let this Archetype know what the 'prev' archetype would be
    // if the component ctype was removed from an entity.
    void AdvisePrev(component_type_id_t ctype, Archetype *arch)
    {
      // assert(prev_.count(ctype) == 0); // should not exist
      assert(arch != nullptr);
      prev_[ctype] = arch;
    }

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

    // Moves all data, the entity membership, and
    // Returns the new archetype
    archetype_t MigrateEntityTo(entity_id_t entity, Archetype *arch, size_t max_entities)
    {
      // To migrate an entity we:
      // 1. move all data in components this archetype has that the other has.
      // 2. remove the entity from this archetype
      // 3. add the entity to the new archetype
      // 4. Return the entity's new archetype so it can be updated externally.

      // 1. Move data. Assumption: the destination Archetype has been set up.
      // TODO: OPTIMISATION: This would be faster if we just had a set, or,
      // we compared archetype_, and just used those keys.
      for (auto item : component_map_)
      {
        if (arch->HasComponent(item.first)) // if the dest arch has this component
        {
          // Get the component metadata
          ComponentMeta meta = component_meta_[item.first];

          // dest may not have the right setup.
          void *dest_data = arch->CreateDataForComponent(item.first, max_entities);

          // Copy the component data.
          uint8_t *dest = static_cast<uint8_t *>(dest_data);
          uint8_t *src = static_cast<uint8_t *>(component_map_[item.first]);
          // I am just going to straight up memcpy it.
          // The offset using entities as indexes is the entity * the data size.
          size_t offset = (entity * meta.DataSize());
          std::memcpy(dest + offset, src + offset, meta.DataSize());
          // std::cout << "Copied " << meta.DataSize() << " bytes for component " << item.first << " from arch " << archetype_ << " to " << arch->archetype_ << std::endl;
        }
      }
      // 2.
      entities_.erase(entity);
      // 3.
      arch->entities_.insert(entity);
      // 4.
      return arch->archetype_;
    }

    // Migrate entities and date to a 'next' Archetype
    archetype_t MigrateEntityToNext(entity_id_t entity, component_type_id_t ctype, size_t max_entities)
    {
      assert(next_.count(ctype) != 0);
      return MigrateEntityTo(entity, next_[ctype], max_entities);
    }
    // Migrate entities and data to a 'prev' Archetype
    archetype_t MigrateEntityToPrev(entity_id_t entity, component_type_id_t ctype, size_t max_entities)
    {
      assert(prev_.count(ctype) != 0);
      return MigrateEntityTo(entity, prev_[ctype], max_entities);
    }
  };

  typedef struct
  {
    size_t allocated_bytes;
    size_t used_bytes;
  } MemStats;

  using ComponentArchMap = std::unordered_map<component_type_id_t, void *>;

  class ECS
  {
  private:
    size_t max_entities_;
    // How many entities do we have?
    size_t num_entities_;
    // for assigning the next entity id.
    entity_id_t current_entity_id_ = 0;
    // entities which have been freed go on this queue.
    std::queue<entity_id_t> free_entities_;
    // for assigning the next component type id.
    component_type_id_t current_component_type_id_ = 0;
    uint8_t used_component_types = 0;

    // Array of which components an entity has.
    archetype_t *entity_index_;
    // Map of archetypes.
    std::unordered_map<archetype_t, Archetype> archetype_map_;
    // Array of component type pow^2 to component metadata
    ComponentMeta component_types_[MAX_COMPONENT_TYPES];

    // Map of archetype to component (data) in that archetype
    // TODO delete this, should be superseded by the atch map.
    // std::unordered_map<archetype_t, ComponentArchMap> components_;

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
      // Create the archetype
      archetype_map_[archetype] = Archetype(archetype, component_types_);
      Archetype *retval = &archetype_map_[archetype];
      // Insert nulls into the component map,
      // Go through each of the 64 bits of the archetype..
      for (uint8_t i = 0; i < MAX_COMPONENT_TYPES; i++)
      {
        // The uint64 value is really important here
        // or it reverts to 32 bit.
        uint64_t one = 1;
        uint64_t cmp = (one << i);
        if ((archetype & cmp) == cmp)
        {
          // Match. Set this key.
          retval->component_map_[i] = nullptr;
        }
      }
      // so these can be initialised when components are added.
      // next, prev, etc: better to fill these
      // in later, when the first migrations take place.
      return *retval;
    }

  public:
    ECS(size_t capacity) : max_entities_(capacity)
    {
      entity_index_ = new archetype_t[max_entities_];
      // This is probably a good start, but you can have
      // far more archetypes than the number of component types.
      archetype_map_.reserve(MAX_COMPONENT_TYPES);
      // components_.reserve(MAX_COMPONENT_TYPES);
    }
    ECS() : ECS(DEFAULT_MAX_ENTITIES) {}

    size_t Count()
    {
      return num_entities_;
    }

    size_t NumArchetypes()
    {
      return archetype_map_.size();
    }

    size_t GetSuperArchetypes(archetype_t archetype, std::vector<Archetype *> *result)
    {
      result->clear();
      for (auto item : archetype_map_)
      {
        // If any existing archetype matches on all
        // the bits of the candidate archetype,
        // add it to the result
        if ((item.first & archetype) == archetype)
        {
          std::cout << "SuperArchetypes, adding " << static_cast<archetype_t>(archetype) << std::endl;
          // TODO this might resize the vector a few times.
          // We could count the number first and reserve()
          result->push_back(&archetype_map_[item.first]);
        }
      }
      return result->size();
    }

    entity_id_t CreateEntity()
    {
      entity_id_t new_eid;
      // entity id of zero is ok (after all it is an index)
      if (free_entities_.size() > 0)
      {
        new_eid = free_entities_.front();
        free_entities_.pop();
      }
      else
      {
        new_eid = current_entity_id_++;
      }
      entity_index_[new_eid] = 0; // empty archetype.
      num_entities_++;
      return new_eid;
    }

    // To remove an entity:
    // The main ecs should know that this entity id is free.
    // The Archetype for this entity should remove it.
    size_t RemoveEntity(entity_id_t entity)
    {
      // TODO: check if it's already free...

      // remove it from the old archetype
      Archetype &old_Archetype = GetArchetypeForEntity(entity);
      old_Archetype.entities_.erase(entity);
      // Clear the entity index at this entity
      entity_index_[entity] = 0; // no archetype
      // Add to the queue of free entities
      free_entities_.push(entity);
      // Return the new size of the total number of entities.
      return --num_entities_;
    }

    Archetype &GetArchetypeForEntity(entity_id_t entity)
    {
      // see if the archetype exists
      archetype_t archetype = entity_index_[entity];
      return GetArchetype(archetype);
    }

    Archetype &GetArchetype(archetype_t archetype)
    {
      // Create the archetype if it doesn't exist.
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
    ComponentMeta GetComponentMeta()
    {
      component_type_id_t c = GetComponentTypeID<T>();
      return component_types_[c];
    }

    template <typename T>
    archetype_t AddComponent(entity_id_t entity, T component)
    {
      // Get the componentid for this component.
      component_type_id_t c = GetComponentType<T>();
      archetype_t old_arch = entity_index_[entity];
      // Result archetype will be...
      archetype_t cmp = 1;

      archetype_t new_arch = entity_index_[entity] | (cmp << c);

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
        new_archetype.AdvisePrev(c, &old_archetype);
        // And new_arch is a next for old_arch
        old_archetype.AdviseNext(c, &new_archetype);

        // Migrate entity from Old to New.
        // This returns the new archetype but we don't use it.
        old_archetype.MigrateEntityToNext(entity, c, max_entities_);
      }
      else
      {
        new_archetype.entities_.insert(entity);
      }

      // If this is the first time a component has been
      // added in this archetype, it will not have an
      // allocated component array.
      // Regardless, add data to new array
      static_cast<T *>(new_archetype.CreateDataForComponent(c, max_entities_))[entity] = component;

      // Update the entity's architecture
      entity_index_[entity] = new_arch;
      return new_arch;
    }

    template <typename T>
    bool HasComponent(entity_id_t entity)
    {
      Archetype &a = GetArchetypeForEntity(entity);
      std::cout << "ECS2::HasComponent - Archetype is " << (int)a.archetype_ << std::endl;
      component_type_id_t c = GetComponentType<T>();
      return a.HasComponent(c);
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
      component_type_id_t c = GetComponentType<T>();
      Archetype &archetype = GetArchetypeForEntity(entity);
      // So this will segfault if:
      //   the entity isn't in the archetype (maybe?)
      //   the archetype doesn't have that component (definintely)
      //   the archetype wasn't set up correctly
      return &static_cast<T *>(archetype.component_map_[c])[entity];
    }
  };

  class Query
  {
  private:
    ECS *ecs_;
    archetype_t arch_;
    size_t ecs_num_archetypes_; // cache the number of archetypes
    std::vector<Archetype *> archetypes_;

    size_t ArchetypesSize()
    {
      size_t retval = 0;
      for (const auto &item : archetypes_)
      {
        retval += item->entities_.size();
      }
      return retval;
    }

  public:
    class Iterator
    {

    public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = entity_id_t;
      using difference_type = std::ptrdiff_t;
      using pointer = const entity_id_t *;
      using reference = const entity_id_t &;

      // Type aliases for readability
      using OuterIter = typename std::vector<Archetype *>::const_iterator;
      using InnerIter = typename std::set<entity_id_t>::const_iterator;

      Iterator(OuterIter outer_it, OuterIter outer_end) : outer_it_(outer_it), outer_end_(outer_end)
      {
        if (outer_it_ != outer_end_)
        {
          inner_it_ = outer_it_.operator*()->entities_.begin();
          advance_to_valid(); // skip empty sets.
        }
      }

      // Dereference operators
      reference operator*() const { return *inner_it_; }
      pointer operator->() const { return &(*inner_it_); }

      // Prefix increment (++it)
      Iterator &operator++()
      {
        ++inner_it_;
        advance_to_valid();
        return *this;
      }

      // Postfix increment (it++)
      Iterator operator++(int)
      {
        Iterator tmp = *this;
        ++(*this);
        return tmp;
      }

      // Comparison operators
      friend bool operator==(const Iterator &a, const Iterator &b)
      {
        if (a.outer_it_ != b.outer_it_)
          return false;
        // If both hit the end, they are equal regardless of inner iterator state
        if (a.outer_it_ == a.outer_end_)
          return true;
        return a.inner_it_ == b.inner_it_;
      }

      friend bool operator!=(const Iterator &a, const Iterator &b)
      {
        return !(a == b);
      }

    private:
      // Declared down here so I can use the 'using pointer=' declaration.
      // Query *query_; // the query we are iterating over.
      OuterIter outer_it_;
      OuterIter outer_end_;
      InnerIter inner_it_;

      // Helper to automatically skip empty nested sets
      void advance_to_valid()
      {
        while ((outer_it_ != outer_end_) && inner_it_ == outer_it_.operator*()->entities_.end())
        {
          ++outer_it_;
          if (outer_it_ != outer_end_)
          {
            inner_it_ = outer_it_.operator*()->entities_.begin();
          }
        }
      }
    };

    Archetype *archetype_;

    Iterator begin()
    {
      // Before we begin, see if we need to recalculate.
      size_t num_archetypes = ecs_->NumArchetypes();
      if (ecs_num_archetypes_ != num_archetypes)
      {
        ecs_num_archetypes_ = num_archetypes;
        std::cout << "ECS2 Query Execute, recalculating membership..." << std::endl;
        // need to recalculate all the subtypes we could be
        ecs_->GetSuperArchetypes(arch_, &archetypes_);
        std::cout << "Membership is: ";
        for (const auto item : archetypes_)
        {
          std::cout << item->archetype_;
        }
        std::cout << std::endl;
      }

      return Iterator(archetypes_.begin(), archetypes_.end());
    }
    Iterator end() { return Iterator(archetypes_.end(), archetypes_.end()); }

    Query(ECS *ecs) : ecs_(ecs), arch_(0), archetype_(nullptr)
    {
      ecs_num_archetypes_ = 0; // this will update on first execute.
    }
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
      arch_ |= (cmp << t);                     // the archetype now becomes...
      archetype_ = &ecs_->GetArchetype(arch_); // the primary arch is now.
      // but others exist. All the archetypes for which this
      // is a sub archetype, i.e. archetypes which are supers of this.
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
    Query &QEntities() { return *query_; }
    virtual void Update(float dt) {}
  };
}

#endif
