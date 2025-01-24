#ifndef __HANDLE_H__
#define __HANDLE_H__

class Handle
{
public:
  uint16_t object_index;
  uint8_t allocator_index;
  template <class T>
  T *get() { return nullptr; }
};
#endif
