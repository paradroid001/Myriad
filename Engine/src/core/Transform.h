#ifndef __TRANSFORM_H_
#define __TRANSFORM_H_
#include "../..//vendor/raylib/src/raylib.h" //quaternion
#include "Component.h"
#include "Types3D.h"
#include "core.h"

namespace Myriad
{
    class MYR_API Transform : public Myriad::Component
    {
      public:
        Transform();
        Transform(Vector3 position);
        ~Transform();

        // Getters
        const Myriad::Vector3 &position() const { return _position; }
        const Quaternion &rotation() const { return _rotation; }
        const Myriad::Vector3 &scale() const { return _scale; }

        // Setters
        void SetPosition(float x, float y, float z);
        void SetRotation(float x, float y, float z, float w);
        void SetScale(float x, float y, float z);

      private:
        Myriad::Vector3 _position;
        Quaternion _rotation;
        Myriad::Vector3 _scale;
    };
} // namespace Myriad
#endif