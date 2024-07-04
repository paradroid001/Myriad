#ifndef MYR_CORE_GAMEOBJECT_H
#define MYR_CORE_GAMEOBJECT_H

#include "core/core.h"
#include "rendering/Renderer.h"

namespace Myriad
{
    class MYR_API GameObject
    {
      protected:
        float x;
        float y;
        float v_x;
        float v_y;

      public:
        GameObject();
        void SetPosition(float posx, float posy);
        void SetVelocity(float velx, float vely);
        virtual void Update(float dt);
        virtual void Draw(Renderer &renderer);
        ~GameObject();
    };
} // namespace Myriad
#endif
