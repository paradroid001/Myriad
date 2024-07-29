#include "Handle.h"
#include "Allocator.h"

template <class T>
T *Handle::Get()
{
  return allocator->GetPtr<T>(index);
}
