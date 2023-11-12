#include "Transform.h"
#include "Component.h"
#include "core.h"

namespace Myriad
{
    Transform::Transform() : Myriad::Component()
    {
        SetComponentData(&_transformData);
        SetPosition(0, 0, 0);
    };
    Transform::Transform(Vector3 position) : Transform::Component()
    {
        this->SetPosition(position.x, position.y, position.z);
    }

    Transform::~Transform(){};

    void Transform::SetPosition(float x, float y, float z)
    {
        this->_transformData.position.x = x;
        this->_transformData.position.y = y;
        this->_transformData.position.z = z;
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