#ifndef MYR_CORE_GAMEOBJECT_H
#define MYR_CORE_GAMEOBJECT_H

#include "core/core.h"

#include "core/component/Transform.h"
#include "core/object/MyrObject.h"
#include "io/Log.h"

//TODO temporary, should go once I add GetComponent<>
#include "core/IUpdateable.h"
#include "core/IDrawable.h"
// #include "rendering/Renderer.h"

namespace Myriad
{
    class MYR_API GameObject : public MyrObject
    {
      protected:
        /*
        float x;
        float y;
        float v_x;
        float v_y;
        */
        Transform *transform_; // GameObjects cache their transform?
        IUpdateable *updater_;
        IDrawable *drawer_;

      public:
        GameObject();
        GameObject(const std::string name);
        virtual ~GameObject();
        Transform &GetTransform() const;
        // Temporary
        IUpdateable *GetUpdater() const;
        IDrawable *GetDrawer() const;

        /*
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

        // virtual void Update(float dt) {}
        // virtual void Draw(Renderer &renderer) {}
        */
    };
} // namespace Myriad
#endif
