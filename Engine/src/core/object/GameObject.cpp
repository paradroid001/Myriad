#include "core/object/GameObject.h"
#include "core/component/Transform.h"
#include "core/object/MyrObjectManager.h"

// TODO temporary, should go once I add GetComponent<>
#include "core/IDrawable.h"
#include "core/IUpdateable.h"

namespace Myriad
{
    GameObject::GameObject(MyrObjectManager *p_mgr)
        : MyrObject(p_mgr), updater_(nullptr), drawer_(nullptr)
    {
        // TODO there's nothing to 'protect' transform component...
        transform_ = AddComponent<Transform>();

        // AddComponent(transform_);
    }

    GameObject::GameObject(const std::string name, MyrObjectManager *p_mgr)
        : MyrObject(name, p_mgr)
    {
        transform_ = AddComponent<Transform>();
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
