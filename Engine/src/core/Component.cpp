#include "Component.h"
#include "GameObject.h"
#include "core.h"

namespace Myriad
{
    Component::Component(){};
    Component::~Component(){};

    void Component::Init(){};
    // GameObject *Component::parent() { return _gameObject; }
    void Component::Enable(){};
    void Component::Disable(){};
    void Component::Exec(){};
    void Component::OnEnable(){};
    void Component::OnDisable(){};
} // namespace Myriad