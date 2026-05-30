#ifndef _MYRIAD_GAME_PHYSICS2D_H_
#define _MYRIAD_GAME_PHYSICS2D_H_

#include "core/core.h"
#include "game/oc/Component.h"

namespace Myriad::ObjectComponent
{
    class MYR_API Physics2D : public Component<Physics2D>
    {
      protected:
        Rect2D rect;

      public:
        // Constructor which sets the right component type.
        // See Component.h
        // Physics2D() : Component(COMPONENT_TYPE_PHYSICS2D) {}
        ~Physics2D() {};
        void Init(Rect2D init_rect);
        inline Rect2D *GetRect2D() { return &rect; }
        void Update(float dt) override;
    };

} // namespace Myriad::ObjectComponent

#endif
