#include "Handle.h"
#include "Allocator.h"
#include <cstdint>
#include <memory>

Allocator::Allocator()
{
  next_index = 0; // this index will be used for an allocator handle.
  // Zero everything
  for (int i = 0; i < MAX_HANDLES; i++)
  {
    memrecords_[i] = NULL;
  }
  // Don't use the first slot.
  next_index += 1;
}
Allocator::~Allocator()
{
  int counter = 0;
  // Don't delete the first slot
  for (int i = 1; i < next_index; i++)
  {
    if (memrecords_[i] != NULL)
    {
      delete memrecords_[i];
      counter += 1;
    }
  }
  // 0 index would be us, the allocator
  if (memrecords_[0] != NULL)
    delete memrecords_[0];
  // We shouldn't need to delete here,
  // This should be triggered by our delete.
  std::cout << "Allocator destructor deleted " << counter << " pointers" << std::endl;
}
/*
Handle *Allocator::MakeHandle(uint16_t index, Allocator *alloc)
{
  return new Handle(index, alloc);
}
*/
