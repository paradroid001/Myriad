#include "GameObject.h"
// #include "MyrEntryPoint.h"
#include "core.h"
#include <iostream>

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
#if MYRIAD_RENDERER == RAYLIB
        // override me
        DrawCircle(_ptransform->position().x, _ptransform->position().y, 20,
                   GREEN);
#endif
    }
} // namespace Myriad
