#include "GameObject.h"
#include "MyrApplication.h" // for myrdata
#include "rendering/Renderer.h"

namespace Myriad
{
    GameObject::GameObject() {}

    GameObject::~GameObject() {}

    void GameObject::SetPosition(float pos_x, float pos_y)
    {
        x = pos_x;
        y = pos_y;
    }

    void GameObject::SetVelocity(float vel_x, float vel_y)
    {
        v_x = vel_x;
        v_y = vel_y;
    }

    void GameObject::Update(float dt) {}

    void GameObject::Draw(Renderer &renderer) {}
} // namespace Myriad
