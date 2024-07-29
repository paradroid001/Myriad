#ifndef MYRIAD_CORE_MYRHANDLE_H
#define MYRIAD_CORE_MYRHANDLE_H

#include "core/core.h"
#include "core/memory/AllocatorProvider.h"
#include "io/Log.h"

namespace Myriad
{
    // This is just a base class so that the allocator
    // has a class to pin the template variants on,
    // for the purpose of deleting them
    class MYR_API HandleBase
    {
      public:
        virtual ~HandleBase(){};
    };

    // A handle is a pointer abstraction
    template <class T> class MYR_API MyrHandle : public HandleBase
    {
      private:
        uint16_t index_;
        // AllocatorProvider *allocator;
        T *ptr_;

      public:
        MyrHandle() : index_(0) {}
        MyrHandle(int idx, T *p) : index_(idx), ptr_(p){};
        ~MyrHandle();
        T *Get() { return ptr_; }
        uint16_t Index() const { return index_; }

        /*
        private:
          T *ptr;
          int *ref_count;

        public:
          MyrHandle() : ptr(0), ref_count(new int(0)) {}
          MyrHandle(T *in) : ptr(in), ref_count(new int(1)) {}

          // Deletes managed obj when ref count is 0
          ~MyrHandle();
          // Copy constructor increments ref count
          MyrHandle(const MyrHandle<T> &rhs);

          // Assignment operator decs lhs ref_count and
          // increments rhs ref count
          MyrHandle<T> operator=(const MyrHandle<T> &rhs);

          // Can just use the handle as a pointer
          T *operator->() const { return ptr; }
          T &operator*() const { return *ptr; }

          T *Get() const { return ptr; }

          const int GetRefCount() { return *ref_count; }

          // Only the allocator should call this.
          void __Destroy()
          {
              *ref_count = -1;
              MyrHandle<T> dummy = *this;
              // now let this go out of scope, trigger the thing.
          }

          const T *ConstPtr()
          {
              return (const T *)ptr;
          } // todo try a const_cast here.

          */
    };

    template <class T> MyrHandle<T>::~MyrHandle()
    {
        MYR_CORE_TRACE("MyrHandle destructor - delete ptr");
        delete ptr_;
    }
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
