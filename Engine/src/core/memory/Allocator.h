#ifndef MYRIAD_CORE_MEMORY_ALLOCATOR_H
#define MYRIAD_CORE_MEMORY_ALLOCATOR_H

#include "core/memory/Allocator.fwd.h"

#include "core/core.h"
#include "core/memory/AllocatorProvider.h"
#include "core/memory/MyrHandle.fwd.h"
#include "io/Log.h"

#include "core/memory/AllocatorQD.h" // we will use the QD allocator

namespace Myriad
{
    class MYR_API Allocator
    {
      private:
        AllocatorProvider<AllocatorQD> *allocator_provider;

      public:
        Allocator() { allocator_provider = new AllocatorQD(); }
        virtual ~Allocator() {}
        virtual bool Init()
        {
            // allocator_provider = new T();
            allocator_provider->Init();
            return true;
        }
        virtual bool Shutdown()
        {
            if (allocator_provider != nullptr)
            {
                allocator_provider->Shutdown();
                delete allocator_provider;
            }
            return true;
        }

        template <class T, typename... Args> MyrHandle<T> Alloc(Args... args)
        {
            MYR_CORE_TRACE("Allocator is allocating");
            return allocator_provider->Alloc<T>(args...);
        }
    };

    // bool Allocator::AllocatorInit(AllocatorProvider *alloc)
    //{
    //     allocator_provider = alloc;
    //     allocator_provider->Init();
    //     return true;
    // }

} // namespace Myriad

#endif
