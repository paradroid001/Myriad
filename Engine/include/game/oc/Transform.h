#ifndef MYRIAD_GAME_TRANSFORM_H
#define MYRIAD_GAME_TRANSFORM_H

#include "core/core.h"
#include "game/oc/Component.h"

namespace Myriad::ObjectComponent
{
    class MYR_API Transform : public Component<Transform>
    {
      protected:
        Vector3 position_;
        Quaternion rotation_;
        Vector3 scale_;

      public:
        // Constructor which sets the right component type.
        // See Component.h
        Transform()
            : Component(), position_({0, 0, 0}), rotation_({0, 0, 0}),
              scale_({1, 1, 1})
        {
        }
        virtual ~Transform();
        void Init(Vector3 pos);
        inline Vector3 GetPosition() const { return position_; }
        inline Quaternion GetRotation() const { return rotation_; }
        inline Vector3 GetScale() const { return scale_; }
        void SetPosition(float x, float y, float z);

        void Update(float dt) override;
    };
} // namespace Myriad::ObjectComponent

#endif
