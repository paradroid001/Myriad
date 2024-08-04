#ifndef MYRIAD_CORE_ALLOCATORPROVIDER_H
#define MYRIAD_CORE_ALLOCATORPROVIDER_H

// Predeclare MyrHandle
#include "core/memory/MyrHandle.fwd.h"

#include "core/memory/AllocatorProvider.fwd.h"

#include "core/Provider.h"
#include "core/core.h"
#include "io/Log.h"
#include <cassert>
#include <iostream>
#include <map>

namespace Myriad
{
    class AllocatorProviderBase : public Provider
    {
      public:
        // virtual uint16_t AssignSlot(void *vptr) = 0;
        virtual void *Ptr(uint16_t index) = 0;
    };

    template <class T>
    class MYR_API AllocatorProvider : public AllocatorProviderBase
    {
      public:
        virtual bool Init() { return true; }
        virtual bool Shutdown() { return true; }
        // So here, C++ doesn't support virtual variadic functions
        // I have to return something.
        template <class U, typename... Args> MyrHandle<U> Alloc(Args... args)
        {
            // NOTE
            // We are using CRTP to get compile time binding to Derived.
            // https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
            //
            // Potentially some compilers may not like it if
            // derived::funcname is the same as funcname, so we use
            // "New" in the derived class instead of Alloc.
            //
            //  We want the derived class, the allocator provider, to handle
            //  all the memory allocation. So we need to pass the type of
            //  object to be instantiated, and the args for the constructor.
            //
            //  The derived class function signature will be:
            //  template <class T, typename... Args> MyrHandle<T> New(Args...
            //  args){...}
            //
            //  The problem is, if we try to call this via the CRTP idiom, i.e.:
            //  (1) static_cast<T*>(this)->New(args...) or
            //  (2) static_cast<T*>(this)->Alloc<U>(args...) we get compile
            //  errors either way - either the compiler can't deduce type T (in
            //  derived) - (1), or the compiler doesn't expect a type argument
            //  U, is confused by <U> (2).
            //
            //  The weird solution is to pass through a dummy argument type T*,
            //  which (I guess?) allows the compiler to know the type of T.
            //  At the moment this is just a dummy T*, which isn't used at all.
            //  I don't know if this works on every compiler.

            U *dummy = nullptr;
            return static_cast<T *>(this)->New(dummy, args...);
        }

        template <class U> MyrHandle<U> Handle(uint16_t index) const
        {
            return static_cast<T *>(this)->At(index);
        }

        virtual void *Ptr(uint16_t index)
        {
            // return static_cast<T *>(this)->Get(index);
            return static_cast<T *>(this)->GetVoid(index);
        }

        /*
        template <class U> U *Ptr(uint16_t index) const
        {
            return static_cast<T *>(this)->Get(index);
        }
        */

        /*
        template <class U> MyrHandle<U> Make(U *ptr)
        {
            // NOTE
            // We are using CRTP to get compile time binding to Derived.
            //
        https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
            //
            // Potentially some compilers may not like it if
            // derived::funcname is the same as funcname,
            // in this case MakeHandle.
            // Could simply make derived function name
            // MakeHandleImplementation and that would be fine,
            // Or rename this function adjust the caller.
            return static_cast<T *>(this)->MakeHandle(ptr);
        }
        */
    };
} // namespace Myriad

#endif
