#ifndef MYRIAD_CORE_MYRCOMPONENT_H
#define MYRIAD_CORE_MYRCOMPONENT_H

#include "core/core.h"
#include <cstdint>

namespace Myriad
{

  typedef enum MyrComponentType : uint8_t // only 255 different components possible...
  {
    NULL_COMPONENT = 0,
    TRANSFORM = 1
  } MyrComponentType;

  class MyrComponent
  {
  protected:
    MYR_ID_t id_;
    MYR_ID_t entity_id_;    // which entity do we belong to?
    MyrComponentType type_; // which component type is this?
  public:
    MyrComponent();
    virtual ~MyrComponent();
    const inline MYR_ID_t GetID() { return id_; }
    const inline MYR_ID_t GetEID() { return entity_id_; }
    const inline MyrComponentType GetType() { return type_; }
    inline void SetEID(MYR_ID_t new_eid) { entity_id_ = new_eid; }
  };
}

#endif
