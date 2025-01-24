#include "game/Transform.h"
#include "io/MyrLogging.h"

namespace Myriad
{
  Transform::Transform() : MyrComponent(), position_({0, 0, 0}), rotation_({0, 0, 0, 0}), scale_({1, 1, 1})
  {
    type_ = MyrComponentType::TRANSFORM;
  }
  Transform::~Transform()
  {
    MYR_CORE_TRACE("Transform destructor: componentid {0}, entityid {1}", id_, entity_id_);
  }

  void Transform::SetPosition(float x, float y, float z)
  {
    position_.x = x;
    position_.y = y;
    position_.z = z;
  }

}
