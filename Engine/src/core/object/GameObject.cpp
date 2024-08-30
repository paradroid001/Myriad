#include "core/object/GameObject.h"
#include "core/component/Transform.h"

// TODO temporary, should go once I add GetComponent<>
#include "core/IDrawable.h"
#include "core/IUpdateable.h"

namespace Myriad
{
    GameObject::GameObject() : MyrObject(), updater_(nullptr), drawer_(nullptr)
    {
        // TODO there's nothing to 'protect' transform component...
        transform_ = new Transform();
        AddComponent(transform_);
    }

    GameObject::GameObject(const std::string name) : MyrObject(name)
    {
        transform_ = new Transform();
        AddComponent(transform_);
    }

    GameObject::~GameObject()
    {
        MYR_CORE_TRACE("Destructing a GameObject");
        RemoveComponent(transform_);
        delete transform_;
    }

    Transform &GameObject::GetTransform() const { return *transform_; }
    IUpdateable *GameObject::GetUpdater() const { return updater_; }
    IDrawable *GameObject::GetDrawer() const { return drawer_; }

} // namespace Myriad
