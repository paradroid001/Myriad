#include "Dot.h"

void Dot::Update()
{
    const Vector3 v = this->_ptransform->position();
    this->_ptransform->SetPosition(v.x + 1, v.y + 1, 0);
}