#include "Component.h"
// #include "GameObject.h"
// #include "core.h"

#include <iostream>
#include <typeindex>
#include <typeinfo>

namespace Myriad
{
    std::type_index Component::_typeid = typeid(ComponentBase);

    void Component::SetType(Component *p)
    {
        _typeid = typeid(p);
        std::cout << "Set typeid to " << _typeid.name() << std::endl;
    }

    Component::Component() : ComponentBase()
    {
        if (_typeid == typeid(ComponentBase))
        {
            Component::SetType(this);
        }
    };
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