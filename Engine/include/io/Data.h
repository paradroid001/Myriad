#ifndef MYRIAD_IO_DATA_H
#define MYRIAD_IO_DATA_H

#include "core/core.h"
#include "io/ISerialisable.h"
#include "json.hpp"

namespace Myriad
{
  using json = nlohmann::json;

  template <typename T>
  class Data : public ISerialisable
  {

    // need to not know about json in client classes. ##
    virtual void to_json() = 0;
    virtual void from_json() = 0;
  };
}
#endif
