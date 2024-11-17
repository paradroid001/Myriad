#ifndef MYRIAD_CORE_COMPONENT_TRANSFORM_H
#define MYRIAD_CORE_COMPONENT_TRANSFORM_H

#include "core/component/MyrComponent.h"
#include "core/core.h"

namespace Myriad
{
    class MYR_API Transform : public MyrComponentBase<Transform>
    {
      protected:
        Vector3 position_;
        Quaternion rotation_;
        Vector3 scale_;

      public:
        Transform()
            : MyrComponentBase(), position_({0, 0, 0}), rotation_({0, 0, 0, 0}),
              scale_({1, 1, 1})
        {
            /*
            I dont think this kind of component structure will
            work. We should really be initialising via the
            MyrComponent constructor here, which means passing
            an owner. But we're never going to want to do that,
            it's way too much of a pain down in the depths of
            a game to be inheriting like that.
            we really just want to set data, and override
            specific methods like Update, Draw, OnEnable, etc.
            */
        }

        virtual inline Vector3 GetPosition() const { return position_; }
        virtual void SetPosition(Vector3 &position)
        {
            position_.x = position.x;
            position_.y = position.y;
            position_.z = position.z;
        }
        virtual void SetPosition(float x, float y, float z)
        {
            position_.x = x;
            position_.y = y;
            position_.z = z;
        }

        // Inherited virtual member from MyrComponent
        virtual bool ReleaseComponent() override { return true; }
    };
} // namespace Myriad

#endif
