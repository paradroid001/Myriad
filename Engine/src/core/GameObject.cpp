#include "GameObject.h"
#include "../../vendor/raylib/src/raylib.h"
#include "core.h"
#include <iostream>

namespace myriad
{
    GameObject::GameObject()
    {
        _ptransform = new Transform();
        Component *c = (Component *)_ptransform;
        AddComponent(*c);
        name = "DefaultName";
    }
    GameObject::~GameObject()
    {
        std::cout << "Destructor for GameObject[" << name << "]" << std::endl;
    }
    int GameObject::AddComponent(Component c)
    {
        components.push_front(c);
        return 0;
    }

    int GameObject::RemoveComponent(Component c) { return 0; }

    Transform GameObject::GetTransform() { return *_ptransform; }

    std::string GameObject::GetName() { return name; }

    void GameObject::SetName(const char *newname)
    {
        name = "This is hardcoded!";
    }

    void GameObject::Update()
    {
        // override me
    }
    void GameObject::Draw()
    {
        // override me
        DrawCircle(_ptransform->position().x, _ptransform->position().y, 20,
                   GREEN);
    }
} // namespace myriad
