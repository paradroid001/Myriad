#ifndef _ALLOCATOR_H_
#define _ALLOCATOR_H_

#include <iostream>
#include "Handle.h"
#define MAX_SLOTS 10

class Allocator
{
public:
  Allocator(){};
  ~Allocator(){};
  Handle alloc()
  {
    if (++_next_slot < MAX_SLOTS)
    {
      std::cout << "Allocating" << std::endl;
      _handles[_next_slot] = new Handle(_next_slot);
      std::cout << "Finished Allocating" << std::endl;
    }
    else
    {
      std::cout << "Out of slots" << std::endl;
    }
    return *_handles[_next_slot];
  };

private:
  // Allocator puts a handle to itself at slot 0?
  // Either way it does not use slot 0.
  Handle *_handles[MAX_SLOTS];
  int _next_slot = 1;
};
#endif
