#include "Component.h"
// #include "GameObject.h"
// #include "core.h"
#include "io/Log.h"

#include <iostream>
#include <typeindex>
#include <typeinfo>

namespace Myriad
{
    std::type_index Component::_typeid = typeid(ComponentBase);

    void Component::SetType(Component *p)
    {
        _typeid = typeid(p);
        MYR_CORE_TRACE("Set typeid to {0}",_typeid.name());
    }

    Component::Component() : ComponentBase()
    {
        if (_typeid == typeid(ComponentBase))
        {
            Component::SetType(this);
        }
        MYR_CORE_TRACE("ComponentCreated");
    };
    Component::~Component()
    {
        MYR_CORE_TRACE("Component Destructor {0}", m_name);
    }

    void Component::Init(){};
    //  GameObject *Component::parent() { return _gameObject; }
    void Component::Enable(){};
    void Component::Disable(){};
    // void Component::Exec(){};
    void Component::OnEnable(){};
    void Component::OnDisable(){};
} // namespace Myriad