#ifndef MYRIAD_CORE_MEMORY_MYRHANDLE_H
#define MYRIAD_CORE_MEMORY_MYRHANDLE_H

#include "core/core.h"
#include "core/memory/MyrHandle.fwd.h" //so myrhandle is defined before

// #include "core/memory/Allocator.h"
#include "core/memory/AllocatorProvider.h" //So I can call allocator->Ptr()

#include "io/Log.h"
#include <cstdint>

namespace Myriad
{
    const MyrHandle_T MYRHANDLE_INVALID_INDEX = 0;
    // A handle is a pointer abstraction, but only gives you an index
    // It does need to know the allocator that allocated it, though.
    template <class T> class MYR_API MyrHandle
    {
      private:
        MyrHandle_T index_;
        AllocatorProviderBase *p_allocator_provider_;

      public:
        MyrHandle()
            : index_(MYRHANDLE_INVALID_INDEX)
        {} // an index of 0 is invalid, that is the address of the allocator.
        MyrHandle(MyrHandle_T idx, AllocatorProviderBase *allocator_provider)
            : index_(idx), p_allocator_provider_(allocator_provider)
        {
        }
        ~MyrHandle()
        { /*MYR_CORE_TRACE("MyrHandle destructor - doing nothing");*/ }
        /*
         * Returns the pointer.
         */
        T *Get() const
        {
            // return static_cast<T *>(p_allocator_provider_->Ptr(index_));
            // Ptr returns a void*, so we have to cast it.
            if (p_allocator_provider_ == nullptr)
                return nullptr;

            return static_cast<T *>(p_allocator_provider_->Ptr(index_));
        }

        inline MyrHandle_T Handle() const { return index_; }

        // -> operator, returns T*
        T *operator->() const { return Get(); }
        // * operator, returns T&
        T &operator*() const { return *Get(); }
    };

    /*
    template <class T> MyrHandle<T>::~MyrHandle()
    {
        // ref_count can be zero if this
        // handle was never assigned to
        // an object.
        // in this case, subtracting can
        // cause a -ve value.
        if (--(*ref_count) <= 0) // not thread safe
        {
            delete ptr;
            delete ref_count;
            ptr = 0;       // null
            ref_count = 0; // null
            MYR_CORE_TRACE("Deleted a myr handle");
        }
        else
        {
            MYR_CORE_TRACE(
                "Did not delete a myr handle because ref count was not 0");
        }
    }

    // Copy constructor increments the ref count.
    // init list is not thread safe.
    template <class T>
    MyrHandle<T>::MyrHandle(const MyrHandle<T> &rhs)
        : ptr(rhs.ptr), ref_count(rhs.ref_count)
    {
        ++(*ref_count);
    }

    // Assignment operator decrements ref_count
    // and increments rhs ref_count
    template <class T>
    MyrHandle<T> MyrHandle<T>::operator=(const MyrHandle<T> &rhs)
    {
        MYR_CORE_TRACE("Inside assignment operator of MyrHandle");
        // no assignment to self
        if (this == &rhs)
        {
            MYR_CORE_WARN("Assignment to self");
            return *this;
        }

        // delete our current implementation (LVAL)
        this->~MyrHandle();

        // adopt new implementation (RVAL)
        ptr = rhs.ptr;
        ref_count = rhs.ref_count; // not thread safe
        ++(*ref_count);

        return *this;
    }
    */

} // namespace Myriad
#endif
