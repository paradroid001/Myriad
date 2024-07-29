#ifndef TESTBED2_ALLOCATOR_H
#define TESTBED2_ALLOCATOR_H

#include <cstdint>
#include "Handle.fwd.h"
#include "Allocator.fwd.h"
#include <memory>
#include <iostream>

#define MAX_HANDLES 4096

class MemRecordBase
{
public:
  virtual ~MemRecordBase(){};
};

template <class T>
class MemRecord : public MemRecordBase
{
private:
  std::unique_ptr<T> ptr;
  const uint16_t index;
  const Handle<T> *phandle;

public:
  // Constructor
  template <typename... Args>
  MemRecord(uint16_t i, Handle<T> *ph, Args... args) : index(i), phandle(ph)
  {
    ptr = std::make_unique<T>(args...);
  };
  // Destructor
  ~MemRecord()
  {
    // don't need to destroy ptr, because it's a unique_ptr.
    // do need to destroy handle.
    delete phandle;
    std::cout << "Destroying MemRecord, deleting a handle." << std::endl;
  }

  // Get
  T *Get(Allocator *allocator) const
  {
    return ptr.get();
  }

  Handle<T> *GetHandle() const
  {
    return phandle;
  }
};

class Allocator
{
private:
  int next_index;
  MemRecordBase *memrecords_[MAX_HANDLES];

public:
  Allocator();
  ~Allocator();

  /* Don't need this function any more
  // We just allocate and hand back the value.
  // The caller can let this go out of scope.
  template <class T>
  Handle<T> MakeHandle(uint16_t index, Allocator *p_allocator)
  {

    return *(new Handle<T>(index, p_allocator));
  }
  */

  //   Alloc an object, get back a handle.
  template <class T, typename... Args>
  Handle<T> Alloc(Args... args)
  {
    // Handle *h = MakeHandle(next_index, this);
    // std::unique_ptr<T> p = std::make_unique<T>(args...);
    // MemRecord<T> *m = new MemRecord<T>(next_index, p);
    Handle<T> *p_handle = new Handle<T>(next_index, this);
    MemRecord<T> *p_memrecord = new MemRecord<T>(next_index, p_handle, args...);

    // T *p = new T(args...);
    memrecords_[next_index] = p_memrecord;
    next_index += 1;
    // return MakeHandle<T>(next_index - 1, this);
    return *p_handle; // return the created handle.
  }

  template <class T>
  T *GetPtr(uint16_t index)
  {
    // MemRecord<T*> mr = static_cast<MemRecord<T*>>(memrecords_[index]);
    // return mr->
    // return NULL;
    MemRecord<T> *m = static_cast<MemRecord<T> *>(memrecords_[index]);
    return m->Get(this);
  }
};

#endif
