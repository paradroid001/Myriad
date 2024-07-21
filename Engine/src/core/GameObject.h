#ifndef MYR_CORE_GAMEOBJECT_H
#define MYR_CORE_GAMEOBJECT_H

#include "core/core.h"
#include "core/object/MyrObject.h"
#include "io/Log.h"
#include "rendering/Renderer.h"

namespace Myriad
{
    class MYR_API GameObject : public MyrObject
    {
      protected:
        float x;
        float y;
        float v_x;
        float v_y;

      public:
        GameObject() : MyrObject() {}
        GameObject(const std::string name) : MyrObject(name) {}
        virtual ~GameObject() { MYR_CORE_TRACE("Destructing a GameObject"); }
        void SetPosition(float posx, float posy)
        {
            x = posx;
            y = posy;
        }
        void SetVelocity(float velx, float vely)
        {
            v_x = velx;
            v_y = vely;
        }
        virtual void Update(float dt) {}
        virtual void Draw(Renderer &renderer) {}
    };
} // namespace Myriad
#endif
