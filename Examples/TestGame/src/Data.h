#ifndef _DATA_H_
#define _DATA_H_

#include <string> //memset?

// A templated double buffer array of type x
template <typename T, std::size_t n>
class Databuffer
{
protected:
  T front_[n];
  T back_[n];

public:
  DataBuffer()
  {
    memset(&front, 0, sizeof(front_));
    memset(&front, 0, sizeof(back_));
  }
  const T read(int n)
  {
    return static_cast<const T>(front_[n]);
  }
  T &write(int n)
  {
    return back[n];
  }
};

#endif
