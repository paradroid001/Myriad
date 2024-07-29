#ifndef TESTBED2_HANDLE_H
#define TESTBED2_HANDLE_H

#include "Handle.fwd.h"
#include "Allocator.h" //so I can actually access allocator functions
#include <cstdint>
#include <iostream>

template <class T>
class Handle
{
public:
  uint16_t index;
  // public uint16_t allocator; //which allocator we are using
  Allocator *allocator;
  Handle(uint16_t i, Allocator *a) : index(i), allocator(a) {}
  ~Handle() { std::cout << "Handle destructor. Doing nothing" << std::endl; }
  T *Get() const
  {
    // return static_cast<T *>(allocator->GetPtr(index));
    // return NULL;
    return allocator->GetPtr<T>(index);
  }
  T *operator->() const { return Get(); }
  T &operator*() const { return *Get(); }
};

#endif
