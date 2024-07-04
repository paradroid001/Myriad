#ifndef MYRIAD_CORE_ALLOCATORSERVICE_H
#define MYRIAD_CORE_ALLOCATORSERVICE_H

#include "IService.h"
#include "MyrHandle.h"
#include "io/Log.h"
#include <cassert>
#include <iostream>
#include <map>

namespace Myriad
{
    class AllocatorService // : public IService
    {
      private:
        std::map<void *, void *> _allocs_map;

      public:
        virtual bool Init();
        virtual bool Shutdown();
        // template <class T> MyrHandle<T> Alloc();
        template <class T, typename... Args> MyrHandle<T> Alloc(Args... args);
    };

    // AllocatorService::~AllocatorService()
    //{
    //     // nothing
    // }

    bool AllocatorService::Init()
    {
        assert(_allocs_map.size() == 0);
        return true;
    }

    bool AllocatorService::Shutdown()
    {
        // delete all the handles.
        for (const auto &[key, value] : _allocs_map)
            MYR_CORE_TRACE("Allocator {0:x} is deleting [{1:x}]={2:x}]",
                           (size_t)this, (size_t)key, (size_t)value);
        // std::cout << "Need to delete: " << '[' << key << "] = " << value
        //           << "; " << std::endl;
        return true;
    }

    template <class T, typename... Args>
    MyrHandle<T> AllocatorService::Alloc(Args... args)
    {
        MYR_CORE_TRACE("Allocator {0:x} is creating a new handle.",
                       (size_t)this);
        MyrHandle<T> *p = new MyrHandle<T>(new T(args...));
        // TODO: thread safe?
        MYR_CORE_TRACE("Inserting into allocs map");
        _allocs_map[(void *)p] = (void *)p->ConstPtr();
        MYR_CORE_TRACE("Returning handle");
        return *p;
    }
} // namespace Myriad

#endif
