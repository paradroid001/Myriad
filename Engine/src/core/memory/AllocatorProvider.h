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
        virtual uint16_t AssignSlot(void *vptr) = 0;
        virtual void *Get(uint16_t index) = 0;
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
            //  So here, I really don't want to call new.
            //  I want to call new in the allocator provider, but
            //  I just can't seem to template it for <class U, typename...
            //  Args>, the reciever doesn't seem to be able to understand the
            //  args, being declared template <class T, typename... Args>
            //  Alloc(Args... args){} And I can't call it, either as: return
            //  static_cast<T *>(this)->Alloc(args...); or return static_cast<T
            //  *>(this)->Alloc<U>(args...);

            U *ptr = new U(args...);
            return this->Make<U>(ptr);
            // return static_cast<T *>(this)->Alloc<U>(args...);
        }

        template <class U> MyrHandle<U> Make(U *ptr)
        {
            // NOTE
            // We are using CRTP to get compile time binding to Derived.
            // https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
            //
            // Potentially some compilers may not like it if
            // derived::funcname is the same as funcname,
            // in this case MakeHandle.
            // Could simply make derived function name
            // MakeHandleImplementation and that would be fine,
            // Or rename this function adjust the caller.
            return static_cast<T *>(this)->MakeHandle(ptr);
        }
    };
} // namespace Myriad

#endif
