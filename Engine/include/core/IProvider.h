#ifndef MYRIAD_CORE_PROVIDER_H
#define MYRIAD_CORE_PROVIDER_H

#include "core/core.h"

namespace Myriad
{
  class IProvider
  {
  public:
    virtual bool Init() = 0;
    virtual bool Shutdown() = 0;
  };
}

#endif
