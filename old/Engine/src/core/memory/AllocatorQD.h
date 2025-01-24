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

#define MAX_HANDLES 16384

    class MemRecordBase
    {
      public:
        virtual ~MemRecordBase() {}
        virtual void *GetVoidRaw() const { return nullptr; }
    };

    template <class T> class MemRecord : public MemRecordBase
    {
      private:
        std::unique_ptr<T> ptr;
        const MyrHandle_T index;
        const MyrHandle<T> *phandle;

      public:
        // Constructor
        template <typename... Args>
        MemRecord(MyrHandle_T i, MyrHandle<T> *ph, Args... args)
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
            // std::cout << "Destroying MemRecord, deleting a handle."
            //           << std::endl;
        }

        // GetRaw
        T *GetRaw() const { return ptr.get(); }

        void *GetVoidRaw() const override
        {
            return static_cast<void *>(ptr.get());
        }

        MyrHandle<T> *GetHandle() const
        {
            return const_cast<MyrHandle<T> *>(phandle);
        }
    };

    class AllocatorQD : public AllocatorProvider<AllocatorQD>
    {
      private:
        int next_index;
        MemRecordBase *memrecords_[MAX_HANDLES];

      public:
        virtual ~AllocatorQD()
        {
            // We shouldn't need to delete anything here. Everything should be
            // null.
            int non_null = 0;
            for (int i = 0; i < MAX_HANDLES; i++)
            {
                if (memrecords_[i] != nullptr)
                    non_null += 1;
            }
            if (non_null > 0)
                MYR_CORE_ERROR("Deleting Allocator, {0} non null ptrs remain.",
                               non_null);
        }

        virtual bool Init()
        {
            next_index = 0; // this index will be used for an allocator handle.
            // Zero everything
            for (int i = 0; i < MAX_HANDLES; i++)
            {
                memrecords_[i] = nullptr;
            }
            // Don't use the first slot.
            next_index += 1;
            return true;
        }
        virtual bool Shutdown()
        {
            int counter = 0;
            // Don't delete the first slot
            for (int i = 1; i <= next_index; i++)
            {
                if (memrecords_[i] != nullptr)
                {
                    delete memrecords_[i];
                    // delete ptrs_[i]; //whoops we have void*, we can't delete
                    memrecords_[i] = nullptr;
                    counter += 1;
                }
            }
            // 0 index would be us, the allocator
            // TODO: so this means the allocator provider deletes itself.
            // the outer, owning 'thing' doesn't need to bother.
            // Deleting ourselves in Shutdown will trigger our destructor,
            // which shouldn't have much to do.
            // if (memrecords_[0] != nullptr)
            //    delete memrecords_[0];

            MYR_CORE_WARN("Allocator Shutdown deleted {0} pointers", counter);

            /*
            MYR_CORE_WARN(
                "Do I still need to fix the allocator? Will it will never "
                "know when handles "
                "are deleted?");
            */
            return true;
        }

        /*
        // New Wave Functions
        // TODO: this function should really be called 'Store', because it
        // officially stores the ptr
        virtual MyrHandle_T AssignSlot(void *vptr) override
        {
            ptrs_[next_index] = vptr;
            next_index++;
            return next_index - 1;
        }

        virtual void *Get(MyrHandle_T index) override
        {
            if (index > next_index) // TODO: also max handles.
                return nullptr;
            return ptrs_[index];
        }
        */

        /*
        template <class T> MyrHandle<T> MakeHandle(T *ptr)
        {
            MyrHandle_T index = AssignSlot(ptr);
            MyrHandle<T> *h = new MyrHandle<T>(index, this);
            return *h; // TODO leak
        }
        */

        // Original Functions
        // template <class T, typename... Args> MyrHandle<T> Alloc(Args...
        // args);
        template <class T, typename... Args>
        MyrHandle<T> New(T *dummyptr, Args... args)
        {
            // MYR_CORE_TRACE("Allocator {0:x} is creating a new handle.",
            //                (size_t)this);
            // TODO: we aren't actually checking
            //      if next_index is < MAX_HANDLES.
            //      so if you allocate too many things,
            //      this segfaults (somewhere, eventually)
            MyrHandle<T> *p_handle = new MyrHandle<T>(next_index, this);

            MemRecord<T> *p_memrecord =
                new MemRecord<T>(next_index, p_handle, args...);
            memrecords_[next_index] = p_memrecord;

            // TODO will it let me exec this code?
            // ptrs_[next_index] = new T(args...);
            next_index += 1;
            return *p_handle; // return the created handle.
            // TODO nothing actually cleans up these handles.
        }
        template <class T> void Delete(MyrHandle<T> handle)
        {
            // myrhandle->Index() holds the index it is at.
            T *dummy = nullptr; //initialise dummy to not get warnings.
            // MyrHandle.Handle() returns the MyrHandle_T
            DeleteIndex(dummy, handle.Handle());
        }
        template <class T> void DeleteIndex(T *dummy, MyrHandle_T index)
        {
            // we need to delete that memrecord.
            //  deleting that memrecord will trigger the actual object's
            //  destructor.
            // then set the slot at index to nullptr
            MemRecord<T> *p_memrecord =
                static_cast<MemRecord<T> *>(memrecords_[index]);
            if (p_memrecord != nullptr)
            {
                delete p_memrecord;
            }
            memrecords_[index] = nullptr; // this is now zeroed
            // TODO: this is where you would do housekeepeing to say 'this slot
            // is now usable'
        }

        // Gets a pointer to the handle (nullptr if out of range)
        template <class T> MyrHandle<T> *At(T *dummy, MyrHandle_T index)
        {
            // TODO this is obviously not completely correct
            if (index <= next_index)
            {
                MemRecordBase *pmrb = memrecords_[index];
                return (static_cast<MemRecord<T> *>(pmrb))->GetHandle();
            }
            return nullptr;
        }

        // Gets the pointer, but I don't know how to call this.
        template <class T> T *Get(MyrHandle_T index)
        {
            // TODO this is obviously not completely correct
            if (index <= next_index)
            {
                return static_cast<MemRecord<T>>(memrecords_[index])->GetRaw();
            }
            return nullptr;
        }

        void *GetVoid(MyrHandle_T index)
        {
            if (index <= next_index)
            {
                return memrecords_[index]->GetVoidRaw();
            }
            return nullptr;
        }

        // template <class T> T *Ptr(MyrHandle_T index);
    };
} // namespace Myriad

#endif
