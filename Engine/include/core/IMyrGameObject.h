#ifndef MYRIAD_CORE_MYRGAMEOBJECT_H
#define MYRIAD_CORE_MYRGAMEOBJECT_H

#include "core/core.h"
#include <cstdint>
#include "MyrObjectManager.h"

// This is the base interface for game objects, no matter how they are backed.
namespace Myriad
{
  // fwd decls
  // class MyrComponent;

  // Types
  // typedef uint16_t MyrComponentType_t;
  // typedef uint64_t MyrObjectID_t;
  // invalid id is MAX of the ID range.
  // #define MYR_OBJECT_INVALID 0xffffffffffffffff
  // typedef uint64_t MyrComponentID_t;

  class MYR_API IMyrGameObject
  {
  public:
    IMyrGameObject() {}
    // virtual destructor
    virtual ~IMyrGameObject() = 0;

    // management
    virtual MYR_ID_t GetID() const = 0;
    virtual void *GetManager() = 0; // const is harder here.

    // Children
    // virtual bool AddChild(MyrObjectID_t childid) = 0;
    // virtual bool RemoveChild(MyrObjectID_t childid) = 0;
    // virtual bool RemoveChildByIndex(uint32_t index) = 0;
    // Components
    // virtual bool AddComponent(MyrComponentID_t componentid) = 0;
    // virtual bool RemoveComponent(MyrComponentID_t component) = 0;
    // virtual MyrComponentID_t GetComponent(MyrComponentType_t component_type) = 0;
  };
}

#endif
