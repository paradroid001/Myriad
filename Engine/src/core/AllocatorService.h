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
        std::map<HandleBase *, void *> _allocs_map;

      public:
        virtual bool Init()
        {
            assert(_allocs_map.size() == 0);
            return true;
        }
        virtual bool Shutdown()
        {
            // delete all the handles.
            for (const auto &[key, value] : _allocs_map)
            {
                // MYR_CORE_TRACE("Allocator {0:x} is deleting [{1:x}]={2:x}]",
                //                (size_t)this, (size_t)key, (size_t)value);

                // Ok so this allocator is a bit of a mess.

                // We can only delete because the handles have the BaseHandle
                // type. At the moment, handles still have a ref count of 1
                // because of the fact that they are assigned here in this map.
                // We manually call their destructor, and they get decremented
                // by one.
                delete key;
            }

            _allocs_map.clear();

            // So handles have one more ref than they should because this
            // allocator holds them all, but even if they didn't, when they self
            // destruct the allocator will never know, unless the handles all
            // know about the allocator.
            MYR_CORE_WARN("Fix the allocator, it will never know when handles "
                          "are deleted.");
            return true;
        }

        template <class T, typename... Args> MyrHandle<T> Alloc(Args... args)
        {
            MYR_CORE_TRACE("Allocator {0:x} is creating a new handle.",
                           (size_t)this);
            MyrHandle<T> *p = new MyrHandle<T>(new T(args...));
            // TODO: thread safe?
            // TODO this allocator actuall has a huge problem.
            //    something screwy is going on with the ref counts.
            //    but also, we aren't retaining the MyrHandle, just the
            //    raw pointer. That's...probably not what we want,
            //    unless the pointer cleans it self up, which it currently
            //    does do in its destructor, in which case there's no
            //    way for the allocator to know that it was deleted.
            //    but ALSO, we then delete the raw pointer, but the MyrHandle
            //    itself is never actually deleted because of the ref-count
            //    problem.
            MYR_CORE_TRACE("Inserting into allocs map");
            _allocs_map[p] = (void *)p->ConstPtr();
            MYR_CORE_TRACE("Returning handle");
            return *p;
        }
    };
} // namespace Myriad

#endif
