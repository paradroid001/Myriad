#ifndef MYRIAD_CORE_MYROBJECTMANAGER_H
#define MYRIAD_CORE_MYROBJECTMANAGER_H

#include "core/config.h"
#include "core/core.h"

namespace Myriad
{
    class MyrObjectManagerBase
    {
      public:
        virtual ~MyrObjectManagerBase() {}
        virtual void OnObjectCreate(MYR_ID_t id, void *ptr) = 0;
    };

    // This is an abstract class which defines the interface
    // for something that manages objects of some type.
    // This allows you to swap in different allocators and obj types
    template <typename ObjType, typename AllocatorType>
    class MyrObjectManager : public MyrObjectManagerBase
    {
      protected:
        AllocatorType *p_allocator_;

      public:
        MyrObjectManager() : MyrObjectManagerBase() {};
        virtual ~MyrObjectManager() {}
        template <class T, typename... Args> MYR_ID_t CreateObject(Args... args)
        {
            T *dummy = 0;
            return p_allocator_->Alloc(dummy, args...);
        }
        virtual void OnObjectCreate(MYR_ID_t id, void *ptr) = 0;
        // virtual void Destroy(T *object) = 0;
        virtual void DestroyObjectById(MYR_ID_t id) {};
        virtual ObjType *GetObject(MYR_ID_t id) { return nullptr; }
        template <class T> T *Get(MYR_ID_t id)
        {
            return static_cast<T *>(GetObject(id));
        }
        // Children - so this is only for things that can have children..
        // virtual bool AddChild(MYR_ID_t parent, MYR_ID_t child) = 0;
        // virtual bool AddChild(MyrGameObject *parent, MyrObjectID_t child);
    };
} // namespace Myriad

#endif
