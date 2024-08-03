#ifndef MYRIAD_CORE_MEMORY_ALLOCATORQD_H
#define MYRIAD_CORE_MEMORY_ALLOCATORQD_H

#include "core/core.h"
#include "core/memory/AllocatorProvider.h"
#include "core/memory/MyrHandle.h"
#include "io/Log.h"
#include <cassert>
#include <map>

/*
  A quick and dirty allocator.
  I'm not worried about performance,
  just pointer safety.
*/

namespace Myriad
{

#define MAX_HANDLES 4096

    class MemRecordBase
    {
      public:
        virtual ~MemRecordBase(){};
    };

    template <class T> class MemRecord : public MemRecordBase
    {
      private:
        std::unique_ptr<T> ptr;
        const uint16_t index;
        const MyrHandle<T> *phandle;

      public:
        // Constructor
        template <typename... Args>
        MemRecord(uint16_t i, MyrHandle<T> *ph, Args... args)
            : index(i), phandle(ph)
        {
            ptr = std::make_unique<T>(args...);
        };
        // Destructor
        ~MemRecord()
        {
            // don't need to destroy ptr, because it's a unique_ptr.
            // do need to destroy handle.
            delete phandle;
            std::cout << "Destroying MemRecord, deleting a handle."
                      << std::endl;
        }

        // Get
        // T *Get(AllocatorProvider *allocator) const { return ptr.get(); }

        MyrHandle<T> *GetHandle() const { return phandle; }
    };

    class AllocatorQD : public AllocatorProvider<AllocatorQD>
    {
      private:
        int next_index;
        // MemRecordBase *memrecords_[MAX_HANDLES];
        void *ptrs_[MAX_HANDLES];

      public:
        virtual bool Init()
        {
            next_index = 0; // this index will be used for an allocator handle.
            // Zero everything
            for (int i = 0; i < MAX_HANDLES; i++)
            {
                ptrs_[i] = nullptr;
                // memrecords_[i] = NULL;
            }
            // Don't use the first slot.
            next_index += 1;
            return true;
        }
        virtual bool Shutdown()
        {
            int counter = 0;
            // Don't delete the first slot
            for (int i = 1; i < next_index; i++)
            {
                // if (memrecords_[i] != NULL)
                if (ptrs_[i] != nullptr)
                {
                    // delete memrecords_[i];
                    // delete ptrs_[i]; //whoops we have void*, we can't delete
                    counter += 1;
                }
            }
            // 0 index would be us, the allocator
            // if (memrecords_[0] != NULL)
            //    delete memrecords_[0];

            // TODO: so this means the allocator provider deletes itself.
            // the outer, owning 'thing' doesn't need to bother.
            if (ptrs_[0] != nullptr)
            {
                // delete ptrs_[0]; //again, this is void *
            }
            // We shouldn't need to delete here,
            // This should be triggered by our delete.
            std::cout << "Allocator destructor deleted " << counter
                      << " pointers" << std::endl;
            MYR_CORE_WARN(
                "Do I still need to fix the allocator? Will it will never "
                "know when handles "
                "are deleted?");
            return true;
        }

        // New Wave Functions
        // TODO: this function should really be called 'Store', because it
        // officially stores the ptr
        virtual uint16_t AssignSlot(void *vptr) override
        {
            ptrs_[next_index] = vptr;
            next_index++;
            return next_index - 1;
        }

        virtual void *Get(uint16_t index) override
        {
            if (index > next_index) // TODO: also max handles.
                return nullptr;
            return ptrs_[index];
        }
        template <class T> MyrHandle<T> MakeHandle(T *ptr)
        {
            uint16_t index = AssignSlot(ptr);
            MyrHandle<T> *h = new MyrHandle<T>(index, this);
            return *h; // TODO leak
        }

        // Original Functions
        // template <class T, typename... Args> MyrHandle<T> Alloc(Args...
        // args);
        template <class T, typename... Args>
        Myriad::MyrHandle<T> Alloc(Args... args)
        {
            MYR_CORE_TRACE("Allocator {0:x} is creating a new handle.",
                           (size_t)this);
            Myriad::MyrHandle<T> *p_handle =
                new Myriad::MyrHandle<T>(next_index, this);

            // MemRecord<T> *p_memrecord =
            //     new Myriad::MemRecord<T>(next_index, p_handle, args...);
            // memrecords_[next_index] = p_memrecord;

            // TODO will it let me exec this code?
            ptrs_[next_index] = new T(args...);

            next_index += 1;
            return *p_handle; // return the created handle.
        }

        // template <class T> T *Ptr(uint16_t index);
    };
} // namespace Myriad

#endif
