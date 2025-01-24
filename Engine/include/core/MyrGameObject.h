#ifndef MYRIAD_CORE_MYRGAMEOBJECT_H
#define MYRIAD_CORE_MYRGAMEOBJECT_H

#include "core/core.h"
#include <cstdint>

// This is the base interface for game objects, no matter how they are backed.
namespace Myriad
{
  // fwd decls
  class MyrComponent;
  class MyrObjectManager;

  // Types
  typedef uint16_t MyrComponentType_t;
  typedef uint64_t MyrObjectID_t;
  // invalid id is MAX of the ID range.
#define MYR_OBJECT_INVALID 0xffffffffffffffff
  typedef uint64_t MyrComponentID_t;

  class MYR_API MyrGameObject
  {
  public:
    MyrGameObject() {}
    // virtual destructor
    virtual ~MyrGameObject() = 0;

    // management
    virtual MyrObjectID_t GetID() const = 0;
    virtual MyrObjectManager *GetManager() = 0; // const is harder here.

    // Children
    // virtual bool AddChild(MyrObjectID_t childid) = 0;
    // virtual bool RemoveChild(MyrObjectID_t childid) = 0;
    // virtual bool RemoveChildByIndex(uint32_t index) = 0;
    // Components
    // virtual bool AddComponent(MyrComponentID_t componentid) = 0;
    // virtual bool RemoveComponent(MyrComponentID_t component) = 0;
    // virtual MyrComponentID_t GetComponent(MyrComponentType_t component_type) = 0;
  };

  class MYR_API MyrObjectManager
  {
  public:
    virtual ~MyrObjectManager() = 0;
    virtual MyrObjectID_t CreateObject() = 0;
    // virtual void Destroy(T *object) = 0;
    virtual void DestroyObjectById(MyrObjectID_t id) = 0;
    virtual MyrGameObject *GetObject(MyrObjectID_t id) = 0;
    // Children
    virtual bool AddChild(MyrObjectID_t parent, MyrObjectID_t child) = 0;
    // virtual bool AddChild(MyrGameObject *parent, MyrObjectID_t child);
  };
}

#endif
