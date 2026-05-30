#ifndef MYRIAD_CORE_ALLOC_H
#define MYRIAD_CORE_ALLOC_H

#include "core/MyrObjectManager.h"
#include "core/core.h"
#include "io/MyrLogging.h"
#include <iterator> //to provide custom iterator
#include <unordered_map>
#include <vector>

namespace Myriad
{

    class AllocatorBase
    {
      public:
        virtual ~AllocatorBase() {};
    };

    // A class that allocates T, using provider U.
    template <typename T, typename U> class Allocator : public AllocatorBase
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

        // The MyrObjectManagerBase will be passed along to
        // the allocator provider: a null pointer will mean
        // the provider will not try to contact the manager on
        // create etc.
        void Init(MYR_ID_t slots, MyrObjectManagerBase *ptr)
        {
            p_allocator_provider_ = new U(slots, ptr);
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

        void Destroy(MYR_ID_t id) { p_allocator_provider_->Destroy(id); }

        T *Get(MYR_ID_t id) { return p_allocator_provider_->Get(id); }
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
            using value_type = T *; // the type being iterated over.
            using iterator_category = std::
                forward_iterator_tag; // see:
                                      // https://internalpointers.com/post/writing-custom-iterators-modern-cpp
            using pointer =
                value_type *; // a pointer to the object being iterated;
            using reference =
                value_type &; // a ref to the object being iterated;
            using difference =
                std::ptrdiff_t; // a data type for difference in iterator steps

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

            friend bool operator==(const Iterator &a, const Iterator &b)
            {
                return a.m_ptr == b.m_ptr;
            }
            friend bool operator!=(const Iterator &a, const Iterator &b)
            {
                return a.m_ptr != b.m_ptr;
            }

          private:
            pointer m_ptr;
        };

      protected:
        MyrObjectManagerBase *p_manager_;
        MYR_ID_t max_slots_;
        MYR_ID_t next_slot_;  // first free slot (i.e. either next unused, or
                              // from a deletion)
        MYR_ID_t used_slots_; // high watermark, num used slots, is index of
                              // first OOB slot.
        MYR_ID_t next_id_;    // monotonically increasing ids.
        std::vector<T *> vector_;
        // the map is of MYR_ID_t to a pair: the slot id, and the actual
        // pointer.
        std::unordered_map<MYR_ID_t, std::pair<MYR_ID_t, T *>> map_;

      public:
        TypeAllocatorDynamic()
            : AllocatorProviderBase(), max_slots_(0), next_slot_(0),
              used_slots_(0), next_id_(0)
        {
        }
        TypeAllocatorDynamic(MYR_ID_t slots, MyrObjectManagerBase *p_man)
            : TypeAllocatorDynamic()
        {
            max_slots_ = slots;
            p_manager_ = p_man;
            vector_.reserve(max_slots_);
            for (MYR_ID_t i = 0; i < max_slots_; i++)
            {
                vector_.push_back(nullptr);
            }
        }
        virtual ~TypeAllocatorDynamic()
        {
            MYR_CORE_TRACE("Allocator Destructor freeing all slots");
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

        Iterator begin()
        {
            return Iterator(&vector_[0]);
        } // address of 0th element
        Iterator end()
        {
            return Iterator(&vector_[used_slots_]);
        } // address of first OOB element

        template <typename Derived, typename... Args>
        MYR_ID_t Alloc(Derived *dummy, Args... args)
        {

            // Find the first free slot and put a new object there.
            // The only reason we wouldn't alloc an object is if we are out of
            // space.
            if (next_slot_ !=
                MYRIAD_INVALID_ID) // make sure we're not off the end.
            {
                // stick this object there.
                T *p_obj = static_cast<T *>(new Derived(args...));
                MYR_ID_t newobjid = next_id_++; // get a new id
                vector_[next_slot_] = p_obj;
                map_[newobjid] = {next_slot_,
                                  p_obj}; // the std::pair is slot, pointer
                // if this is a new slot use:
                // increment next_slot_ and used_slots_
                if (next_slot_ == used_slots_)
                {
                    next_slot_++;
                    used_slots_++;
                }
                // if backfilling (i.e. filling a hole from delete):
                //- update next_slot_ to next nullptr slot.
                //- don't increase used_slots_ (high water mark)
                else if (next_slot_ < used_slots_)
                {
                    MYR_ID_t pos =
                        next_slot_ +
                        1; // start from slot after just allocated one
                    while (vector_[pos] != nullptr)
                    {
                        pos += 1;
                    }
                    next_slot_ =
                        pos; // next slot should now be first null slot.
                }
                if (next_slot_ > used_slots_)
                {
                    MYR_CORE_ERROR("Slot allocation has gone wrong");
                }
                // Notify the manager, if we have one
                if (p_manager_ != nullptr)
                {
                    p_manager_->OnObjectCreate(newobjid, p_obj);
                }
                return newobjid;
            }
            return MYRIAD_INVALID_ID;
        }
        void Destroy(MYR_ID_t id)
        {
            auto it = map_.find(id);
            if (it != map_.end())
            {
                MYR_ID_t slotnum = it->second.first;
                T *ptr = it->second.second;
                // we found the record. Delete it.
                delete ptr;
                // set the slot in the vector equal to nullptr
                vector_[slotnum] = nullptr;
                // if that slot num is lower than next_slot_, adjust next_slot:
                if (slotnum < next_slot_)
                    next_slot_ = slotnum;
                // erase this record from the map.
                map_.erase(it);
            }
            else
            {
                MYR_CORE_ERROR("No mem record exists for id {0}", id);
            }
        }

        T *Get(MYR_ID_t id)
        {
            auto it = map_.find(id);
            if (it != map_.end())
            {
                return it->second.second; // ptr is the second in the pair
            }
            return nullptr;
        }
    };

} // namespace Myriad

#endif
