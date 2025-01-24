#ifndef __ALLOCATOR_H__
#define __ALLOCATOR_H__

#include <vector>

template <typename T>
class Allocator
{
protected:
  std::vector<T *> allocations;

public:
  ~Allocator()
  {
    for (T *p_obj : allocations)
    {
      delete p_obj;
    }
  }

  template <class U, typename... Args>
  unsigned int Alloc(Args... args)
  {
    U *u = new U(args...);
    allocations.push_back(static_cast<T *>(u));
    std::cout << "Allocating: " << allocations.size() - 1 << std::endl;
    return allocations.size() - 1;
  }
  template <class U>
  U *get(unsigned int index)
  {
    if (index <= (allocations.size() - 1))
    {
      return static_cast<U *>(allocations[index]);
    }
    return nullptr;
  }
};

#endif
