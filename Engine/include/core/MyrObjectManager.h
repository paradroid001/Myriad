#ifndef MYRIAD_CORE_MYROBJECTMANAGER_H
#define MYRIAD_CORE_MYROBJECTMANAGER_H

#include "core/core.h"
#include "core/config.h"

namespace Myriad
{
  // This is an abstract class which defines the interface
  // for something that manages objects of some type.
  // This allows you to swap in different allocators and obj types
  template <typename ObjType, typename AllocatorType>
  class MyrObjectManager
  {
  protected:
    AllocatorType *p_allocator;

  public:
    MyrObjectManager() {};
    virtual ~MyrObjectManager() {}
    template <class T, typename... Args>
    MYR_ID_t CreateObject(Args... args)
    {
      T *dummy = 0;
      return p_allocator->Alloc(dummy, args...);
    }
    // virtual void Destroy(T *object) = 0;
    virtual void DestroyObjectById(MYR_ID_t id) {};
    virtual ObjType *GetObject(MYR_ID_t id) { return nullptr; }
    // Children - so this is only for things that can have children..
    // virtual bool AddChild(MYR_ID_t parent, MYR_ID_t child) = 0;
    // virtual bool AddChild(MyrGameObject *parent, MyrObjectID_t child);
  };
}

#endif
