#include "game/oc/Transform.h"
#include "io/MyrLogging.h"
#include "core/MyrProfiling.h"

namespace Myriad::ObjectComponent
{
  Transform::Transform() : Component(), position_({0, 0, 0}), rotation_({0, 0, 0}), scale_({1, 1, 1})
  {
    // type_ = MyrComponentType::TRANSFORM;
  }
  Transform::~Transform()
  {
    MYR_CORE_TRACE("Transform destructor");
  }

  void Transform::Init(Vector3 pos)
  {
    position_ = pos;
    SetUpdateable(true);
    SetRenderable(false);
  }

  void Transform::SetPosition(float x, float y, float z)
  {
    position_.x = x;
    position_.y = y;
    position_.z = z;
  }

  void Transform::Update(float dt)
  {
    // nothing for now.
  }
}
