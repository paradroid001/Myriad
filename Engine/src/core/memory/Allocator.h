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
            //MYR_CORE_TRACE("Allocator is allocating");
            return allocator_provider->Alloc<T>(args...);
        }
        template <class T> void Dealloc(MyrHandle<T> handle)
        {
            MYR_CORE_TRACE("Allocator is deallocating");
            return allocator_provider->Dealloc<T>(handle);
        }

        // Just get the lite handle.
        template <class T, typename... Args>
        MyrHandle_T AllocIndex(Args... args)
        {
            MyrHandle<T> handle = Alloc(args...);
            return handle.Index();
        }
        template <class T> void DeallocIndex(MyrHandle_T index)
        {
            allocator_provider->DeallocIndex<T>(index);
        }

        template <class T> T *At(MyrHandle_T index)
        {
            // TODO this is about the most inefficient way you could do this..
            MyrHandle<T> handle = allocator_provider->Handle<T>(index);
            return handle.Get();
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
