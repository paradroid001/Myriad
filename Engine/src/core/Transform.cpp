#include "Transform.h"
#include "Component.h"
#include "core.h"

namespace myriad
{
    Transform::Transform() : Component() { SetPosition(0, 0, 0); };
    Transform::Transform(Vector3 position) : Component()
    {
        this->SetPosition(position.x, position.y, position.z);
    }
    Transform::~Transform(){};
    void Transform::SetPosition(float x, float y, float z)
    {
        this->_position.x = x;
        this->_position.y = y;
        this->_position.z = z;
    }
    void Transform::SetRotation(float x, float y, float z, float w)
    {
        // TODO implement me
    }
    void Transform::SetScale(float x, float y, float z)
    {
        // TODO implement me
    }

} // namespace myriad