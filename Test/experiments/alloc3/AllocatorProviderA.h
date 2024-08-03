#ifndef TESTBED3_ALLOCATOR_PROVIDER_A_H
#define TESTBED3_ALLOCATOR_PROVIDER_A_H
// Array implementation
#include "AllocatorProvider.h"
#include <cstdint>

#define MAX_PTRS 200

class AllocatorProviderA : public AllocatorProvider<AllocatorProviderA>
{
private:
  void *ptrs_[MAX_PTRS]; // 200 pointers...
  uint16_t next_slot;

public:
  AllocatorProviderA() : AllocatorProvider(), next_slot(0) {}
  virtual ~AllocatorProviderA() {}

  // MakeHandle
  template <class T>
  Handle<T> *MakeHandle(T *ptr)
  {
    // uint16_t index = AssignSlot(ptr);
    return new Handle<T>(this, ptr);
  }

  // This is a test
  template <class T, typename... Args>
  T *MakePtr(Args... args)
  {
    return new T(args...);
  }

  // And this is another test
  template <class T, typename... Args>
  Handle<T> *AllocImpl(Args... args)
  {
    return new Handle<T>(this, new T(args...));
  }

  // AssignSlot
  uint16_t AssignSlot(void *vptr) override
  {
    ptrs_[next_slot] = vptr;
    uint16_t ret = next_slot;
    next_slot++;
    return ret;
  }

  // GET
  void *Get(uint16_t index)
  {
    if (index < MAX_PTRS)
      return ptrs_[index];
    return NULL;
  }
};

#endif
