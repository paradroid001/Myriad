#include "game/oc/Transform.h"
#include "core/MyrProfiling.h"
#include "io/MyrLogging.h"

namespace Myriad::ObjectComponent
{
    Transform::~Transform() { MYR_CORE_TRACE("Transform destructor"); }

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
} // namespace Myriad::ObjectComponent
