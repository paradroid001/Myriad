#ifndef TESTBED3_HANDLE_H
#define TESTBED3_HANDLE_H

#include <cstdint>
#include "Handle.fwd.h"
#include "AllocatorProvider.h"

class HandleBase
{
protected:
  uint16_t index_;
};

template <class T>
class Handle : public HandleBase
{
private:
  AllocatorProviderBase *allocator_provider_;

public:
  Handle(AllocatorProviderBase *allocator_provider, T *ptr) : HandleBase(), allocator_provider_(allocator_provider)
  {
    index_ = allocator_provider_->AssignSlot(ptr);
  }
  T *Get()
  {
    return static_cast<T *>(allocator_provider_->Get(index_));
  }
};

#endif
