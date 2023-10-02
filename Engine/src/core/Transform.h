#ifndef __TRANSFORM_H_
#define __TRANSFORM_H_
#include "../..//vendor/raylib/src/raylib.h"
#include "Component.h"
#include "core.h"

namespace myriad
{
    class MYR_API Transform : public Component
    {
      public:
        Transform();
        Transform(Vector3 position);
        ~Transform();

        // Getters
        const Vector3 &position() const { return _position; }
        const Quaternion &rotation() const { return _rotation; }
        const Vector3 &scale() const { return _scale; }

        // Setters
        void SetPosition(float x, float y, float z);
        void SetRotation(float x, float y, float z, float w);
        void SetScale(float x, float y, float z);

      private:
        Vector3 _position;
        Quaternion _rotation;
        Vector3 _scale;
    };
} // namespace myriad
#endif