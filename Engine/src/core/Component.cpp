#include "Component.h"
// #include "GameObject.h"
// #include "core.h"

#include <iostream>

namespace Myriad
{
    Component::Component() : ComponentBase(){};
    Component::~Component()
    {
        std::cout << "Component Destructor" << std::endl;
    }

    void Component::Init(){};
    //  GameObject *Component::parent() { return _gameObject; }
    void Component::Enable(){};
    void Component::Disable(){};
    // void Component::Exec(){};
    void Component::OnEnable(){};
    void Component::OnDisable(){};
} // namespace Myriad