#ifndef MYRIAD_CORE_ALLOC_H
#define MYRIAD_CORE_ALLOC_H

#include "core/core.h"
#include <unordered_map>
#include <vector>
#include <iterator> //to provide custom iterator

namespace Myriad
{

  class AllocatorBase
  {
  public:
    virtual ~AllocatorBase() {};
  };

  // A class that allocates T, using provider U.
  template <typename T, typename U>
  class Allocator : public AllocatorBase
  {
  private:
    U *p_allocator_provider_;

  public:
    Allocator() : AllocatorBase() {}
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

    // We have a Derived template param - we want this because
    // even though the allocator 'type' is already set (T, and U<T>),
    // we may still want to create items of derived types of T.
    // But there's no real way to send that over to the
    //  allocator provider (which is also a class template):
    //  p_allocator_provider_->Alloc<Derived>(args...) doesn't seem to work.
    // Instead, I provide a 'dummy' pointer to Derived, which at least
    // lets the compiler deduce the parameters.
    // If there's a better way of doing this, then great.
    template <typename Derived, typename... Args>
    MYR_ID_t Alloc(Args... args)
    {
      Derived *dummy = 0;
      return p_allocator_provider_->Alloc(dummy, args...);
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

  class AllocatorProviderBase
  {
  public:
    virtual ~AllocatorProviderBase() {}
  };

  // This class will alloc dynamic objects, i.e it will
  // use the heap for each allocated object, and they will
  // not be contiguous in memory.
  template <typename T>
  class TypeAllocatorDynamic : public AllocatorProviderBase
  {
    struct Iterator
    {
      using value_type = T *;                              // the type being iterated over.
      using iterator_category = std::forward_iterator_tag; // see: https://internalpointers.com/post/writing-custom-iterators-modern-cpp
      using pointer = value_type *;                        // a pointer to the object being iterated;
      using reference = value_type &;                      // a ref to the object being iterated;
      using difference = std::ptrdiff_t;                   // a data type for difference in iterator steps

      // CONSTRUCTORS
      // All iterators must be
      // - constructible
      Iterator(pointer ptr) : m_ptr(ptr) {} // initialise m_ptr
      // - copy-constructible
      // - copy-assignable
      // - destructible and
      // - swappable
      // These are provided by the compiler

      // OPERATORS
      // Dereference
      reference operator*() const { return *m_ptr; }
      pointer operator->() { return m_ptr; }
      // Prefix increment
      Iterator &operator++()
      {
        m_ptr++;
        return *this;
      }
      // Postfix increment
      Iterator operator++(int)
      {
        Iterator tmp = *this;
        ++(*this);
        return tmp;
      }

      friend bool operator==(const Iterator &a, const Iterator &b) { return a.m_ptr == b.m_ptr; }
      friend bool operator!=(const Iterator &a, const Iterator &b) { return a.m_ptr != b.m_ptr; }

    private:
      pointer m_ptr;
    };

  protected:
    MYR_ID_t max_slots_;
    MYR_ID_t next_slot_;
    MYR_ID_t used_slots_;
    MYR_ID_t next_id_;
    std::vector<T *> vector_;
    std::unordered_map<MYR_ID_t, T *> map_;

  public:
    TypeAllocatorDynamic() : AllocatorProviderBase(), max_slots_(0), next_slot_(0), used_slots_(0), next_id_(0)
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

    Iterator begin() { return Iterator(&vector_[0]); }        // address of 0th element
    Iterator end() { return Iterator(&vector_[next_slot_]); } // address of first OOB element

    template <typename Derived, typename... Args>
    MYR_ID_t Alloc(Derived *dummy, Args... args)
    {
      // find the first free slot
      if (next_slot_ != MYRIAD_INVALID_ID)
      {
        // stick this object there.
        T *p_obj = static_cast<T *>(new Derived(args...));
        vector_[next_slot_] = p_obj;
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
