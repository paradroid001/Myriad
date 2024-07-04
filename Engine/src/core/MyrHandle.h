#ifndef MYRIAD_CORE_MYRHANDLE_H
#define MYRIAD_CORE_MYRHANDLE_H

#include "core.h"
#include "io/Log.h"

namespace Myriad
{
    // A handle is a pointer abstraction
    template <class T> class MYR_API MyrHandle
    {
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
        T *operator->() { return ptr; }
        T &operator*() { return *ptr; }

        const T *ConstPtr() { return (const T *)ptr; }
    };

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
            ptr = 0;
            ref_count = 0;
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

} // namespace Myriad
#endif
