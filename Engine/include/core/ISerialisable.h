#ifndef MYRIAD_CORE_SERIALISABLE_H
#define MYRIAD_CORE_SERIALISABLE_H

#include "json.hpp"

namespace Myriad
{
  class ISerialisable
  {
  public:
    virtual void Serialise() = 0;
    virtual void DeSerialise() = 0;
  };

  /*
  template <typename T>
  void to_json(json &j, const T &t)
  {
    static_cast<const ISerialisable<T> &>(t).to_json(j, t);
  }
  template <typename T>
  void from_json(const json &j, T &t)
  {
    (static_cast<ISerialisable<T> &>(t))->from_json(j, t);
  }
  */
}

#endif
