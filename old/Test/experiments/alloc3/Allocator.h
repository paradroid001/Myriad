#ifndef TESTBED3_ALLOCATOR_H
#define TESTBED3_ALLOCATOR_H

#include <iostream>
#include "Handle.h"
#include "AllocatorProvider.h"
#include "AllocatorProviderA.h"

class Allocator
{
private:
  AllocatorProvider<AllocatorProviderA> *allocator_provider;

public:
  Allocator()
  {
    // allocator_provider = new AllocatorProvider();
    allocator_provider = new AllocatorProviderA();
  }
  virtual ~Allocator()
  {
    // delete the allocator provider...
    if (allocator_provider != nullptr)
    {
      delete allocator_provider;
    }
  }

  template <class T, typename... Args>
  Handle<T> *Alloc(Args... args)
  {
    std::cout << "About to alloc a handle" << std::endl;
    return allocator_provider->Alloc<T>(args...);
  }

  template <class T, typename... Args>
  Handle<T> *Make(Args... args)
  {
    // return new Handle<T>(new T(args...));
    std::cout << "About to make a handle" << std::endl;
    return allocator_provider->Make<T>(new T(args...));
  }
};

#endif
