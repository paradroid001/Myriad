#ifndef MYRIAD_CORE_MYRENTITY_H
#define MYRIAD_CORE_MYRENTITY_H

#include "core/core.h"
#include <cstdint>

namespace Myriad
{
  class MYR_API MyrEntity
  {
  protected:
    MYR_ID_t id_;

  public:
    MyrEntity();
    virtual ~MyrEntity();
    // todo: this potentially orphans a bunch of components
    inline void SetID(MYR_ID_t newid) { id_ = newid; }
    inline MYR_ID_t GetID() const { return id_; }
  };
}

#endif
