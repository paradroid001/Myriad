#include "Transform.h"
#include "Component.h"
#include "core.h"

namespace Myriad
{
    Transform::Transform() : Myriad::Component() { SetPosition(0, 0, 0); };
    Transform::Transform(Vector3 position) : Myriad::Component()
    {
        this->SetPosition(position.x, position.y, position.z);
    }

    Transform::~Transform(){};

    void Transform::SetPosition(float x, float y, float z)
    {
        this->_internalData.position.x = x;
        this->_internalData.position.y = y;
        this->_internalData.position.z = z;
    }

    void Transform::SetRotation(float x, float y, float z, float w)
    {
        // TODO implement me
    }

    void Transform::SetScale(float x, float y, float z)
    {
        // TODO implement me
    }

} // namespace Myriad