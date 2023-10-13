#include "GameObject.h"
// #include "MyrEntryPoint.h"
#include "core.h"
#include <iostream>
// Include the config
#include "MyriadConfig.h"

namespace Myriad
{
    GameObject::GameObject()
    {
        _ptransform = new Myriad::Transform();
        Myriad::Component *c = (Myriad::Component *)_ptransform;
        AddComponent(*c);
        name = "DefaultName";
    }
    GameObject::~GameObject()
    {
        std::cout << "Destructor for GameObject[" << name << "]" << std::endl;
    }
    int GameObject::AddComponent(Myriad::Component c)
    {
        components.push_front(c);
        return 0;
    }

    int GameObject::RemoveComponent(Myriad::Component c) { return 0; }

    Myriad::Transform *const GameObject::GetTransform()
    {
        return (Transform *const)_ptransform;
    }

    std::string GameObject::GetName() { return name; }

    void GameObject::SetName(const char *newname)
    {
        name = "This is hardcoded!";
    }

    void GameObject::Update()
    {
        //   override me
    }

    void GameObject::Draw()
    {
        Color c = GREEN;
        MyrColour mc;
        mc.r = c.r;
        mc.g = c.g;
        mc.b = c.b;
        mc.a = c.a;
        // override me
        this->DrawCircleShape(_ptransform->position().x,
                              _ptransform->position().y, 20, mc);
    }

    void GameObject::DrawCircleShape(float x, float y, float r, MyrColour c)
    {
#if MYRIAD_RENDERER == RAYLIB
        DrawCircle(x, y, r, {c.r, c.g, c.b, c.a});
#endif
    }
} // namespace Myriad
