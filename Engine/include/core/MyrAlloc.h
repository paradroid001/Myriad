#ifndef MYRIAD_CORE_ALLOC_H
#define MYRIAD_CORE_ALLOC_H

#include "core/core.h"
#include <unordered_map>
#include <vector>

namespace Myriad
{
  // A class that allocates T, using provider U.
  template <typename T, typename U>
  class Allocator
  {
  private:
    U *p_allocator_provider_;

  public:
    virtual ~Allocator()
    {
      if (p_allocator_provider_ != nullptr)
      {
        delete p_allocator_provider_;
      }
    }
    void Init(MYR_ID_t slots)
    {
      p_allocator_provider_ = new U(slots);
    }
    template <typename... Args>
    MYR_ID_t Alloc(Args... args)
    {
      return p_allocator_provider_->Alloc(args...);
    }
    void Destroy(MYR_ID_t id)
    {
      p_allocator_provider_->Destroy(id);
    }
    T *Get(MYR_ID_t id)
    {
      return p_allocator_provider_->Get(id);
    }
  };

  // This class will alloc dynamic objects, i.e it will
  // use the heap for each allocated object, and they will
  // not be contiguous in memory.
  template <typename T>
  class MYR_API TypeAllocatorDynamic
  {
  protected:
    MYR_ID_t max_slots_;
    MYR_ID_t next_slot_;
    MYR_ID_t used_slots_;
    MYR_ID_t next_id_;
    std::vector<T *> vector_;
    std::unordered_map<MYR_ID_t, T *> map_;

  public:
    TypeAllocatorDynamic() : max_slots_(0), next_slot_(0), used_slots_(0), next_id_(0)
    {
    }
    TypeAllocatorDynamic(MYR_ID_t slots) : TypeAllocatorDynamic()
    {
      max_slots_ = slots;
      vector_.reserve(max_slots_);
      for (MYR_ID_t i = 0; i < max_slots_; i++)
      {
        vector_.push_back(nullptr);
      }
    }
    virtual ~TypeAllocatorDynamic()
    {
      for (MYR_ID_t i = 0; i < vector_.size(); i++)
      {
        if (vector_[i] != nullptr)
        {
          delete vector_[i];
        }
      }
      vector_.clear();
      map_.clear();
      used_slots_ = 0;
      next_slot_ = 0;
      // next_id = 0;
    }

    template <typename... Args>
    MYR_ID_t Alloc(Args... args)
    {
      // find the first free slot
      if (next_slot_ != MYRIAD_INVALID_ID)
      {
        // stick this object there.
        vector_[next_slot_] = new T(args...);
        map_[next_slot_] = vector_[next_slot_];
        MYR_ID_t ret = next_id_;
        next_id_++;
        next_slot_++;
        used_slots_++;
        return ret;
      }
      return MYRIAD_INVALID_ID;
    }
    void Destroy(MYR_ID_t id)
    {
      // TODO
      // update the vector
      // update the map
    }
    T *Get(MYR_ID_t id)
    {
      if (id < max_slots_ && id < used_slots_)
      {
        return map_[id];
      }
      return nullptr;
    }
  };

}

#endif
