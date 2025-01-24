#ifndef MYRIAD_GAME_TRANSFORM_H
#define MYRIAD_GAME_TRANSFORM_H

#include "core/core.h"
#include "core/MyrComponent.h"

namespace Myriad
{
  class Transform : public MyrComponent
  {
  protected:
    Vector3 position_;
    Quaternion rotation_;
    Vector3 scale_;

  public:
    Transform();
    virtual ~Transform();
    inline Vector3 GetPosition() const { return position_; }
    inline Quaternion GetRotation() const { return rotation_; }
    inline Vector3 GetScale() const { return scale_; }
    void SetPosition(float x, float y, float z);
  };
}

#endif
