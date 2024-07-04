#ifndef _HANDLE_H_
#define _HANDLE_H_

#include <iostream>
// #include "Allocator.h"

class Handle
{
public:
  // Another implementation I have seen has
  // the ref count as an int*.
  // I don't know why.
  Handle(int slot) : slot_index(slot), ref_count(0){};

  // Copy Constructor
  // Increments the ref count.
  // Init lists are not thread safe.
  Handle(const Handle &rhs) : slot_index(rhs.slot_index), ref_count(rhs.ref_count + 1)
  {
    // we already incremented the ref count.
    std::cout << "Incremented: Handle now has " << ref_count << "refs" << std::endl;
  }

  ~Handle()
  {
    if (--(ref_count) <= 0)
    {
      std::cout << "Decremented: Handle has zero refs, deallocating" << std::endl;
      // tell the allocator to delete/free the alloced object.
    }
    else
    {
      std::cout << "Decremented: Handle still has " << ref_count << " refs" << std::endl;
    }
  };

  // assignment operator
  Handle operator=(const Handle &rhs)
  {
    std::cout << "Handle assignment!" << std::endl;
    return rhs;
  }

private:
  int slot_index;
  int ref_count;
};
#endif
