#ifndef __TRANSFORM_H_
#define __TRANSFORM_H_

#include "Component.h"
#include "Types3D.h"
#include "core.h"

namespace Myriad
{
    struct TransformData : ComponentData
    {
        Myriad::Vector3 position;
        Myriad::Quaternion rotation;
        Myriad::Vector3 scale;
    };

    class MYR_API Transform : public Myriad::Component
    {
      public:
        Transform();
        Transform(Myriad::Vector3 position);
        ~Transform();

        // Getters
        const Myriad::Vector3 &position() const
        {
            return _transformData.position;
        }
        const Myriad::Quaternion &rotation() const
        {
            return _transformData.rotation;
        }
        const Myriad::Vector3 &scale() const { return _transformData.scale; }

        // Setters
        void SetPosition(float x, float y, float z);
        void SetRotation(float x, float y, float z, float w);
        void SetScale(float x, float y, float z);

        // virtual ComponentData *Data() { return &_internalData; }

      protected:
        TransformData _transformData;
    };
} // namespace Myriad
#endif