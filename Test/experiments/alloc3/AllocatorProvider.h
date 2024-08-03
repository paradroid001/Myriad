#ifndef TESTBED3_ALLOCATOR_PROVIDER_H
#define TESTBED3_ALLOCATOR_PROVIDER_H

#include "Handle.h"
#include <cstdint>
#include <cassert>
#include <iostream>

class AllocatorProviderBase
{
public:
  virtual uint16_t AssignSlot(void *vptr) = 0;
  virtual void *Get(uint16_t index) = 0;
};

template <class T>
class AllocatorProvider : public AllocatorProviderBase
{

public:
  AllocatorProvider(){};
  virtual ~AllocatorProvider(){};

  template <class U, typename... Args>
  Handle<U> *Alloc(Args... args)
  {
    // U *ptr = static_cast<T *>(this)->MakePtr<U>(args...);

    // NOTE
    //  So here, I really don't want to call new.
    //  I want to call new in the allocator provider, but
    //  I just can't seem to template it for <class U, typename... Args>,
    //  the reciever doesn't seem to be able to understand the args,
    //  being declared template <class T, typename... Args> Alloc(Args... args){}
    //  And I can't call it, either as:
    //  return static_cast<T *>(this)->Alloc(args...);
    //  or
    //  return static_cast<T *>(this)->Alloc<U>(args...);

    U *ptr = new U(args...);
    return this->Make<U>(ptr);
    // return static_cast<T *>(this)->Alloc<U>(args...);
  }

  template <class U>
  Handle<U> *Make(U *ptr)
  {
    // NOTE
    // We are using CRTP to get compile time binding to Derived.
    // https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
    //
    // Potentially some compilers may not like it if
    // derived::funcname is the same as funcname,
    // in this case MakeHandle.
    // Could simply make derived function name
    // MakeHandleImplementation and that would be fine,
    // Or rename this function adjust the caller.
    return static_cast<T *>(this)->MakeHandle(ptr);
  }

  // Only the derived classes will inherit these.
  // virtual uint16_t AssignSlot(void *vptr) = 0;
  // virtual void *Get(uint16_t index) = 0;
};

#endif
