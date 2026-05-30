#include "game/oc/Physics2D.h"
#include "game/oc/Component.h"
#include "game/oc/GameObject.h"
#include "game/oc/Transform.h"

namespace Myriad::ObjectComponent
{
    void Physics2D::Init(Rect2D init_rect)
    {
        rect = init_rect;
        SetUpdateable(true);
        SetRenderable(false);
        MYR_CORE_TRACE("Physics Component {0}, rect = {1},{2},{3},{4}", id_,
                       rect.pos.x, rect.pos.y, rect.size.x, rect.size.y);
    }
    void Physics2D::Update(float dt)
    {
        // Keep in sync with the parent gameobject.
        Transform *transform = owner_->GetComponent<Transform>();
        rect.pos = {transform->GetPosition().x, transform->GetPosition().y};
    }
} // namespace Myriad::ObjectComponent
