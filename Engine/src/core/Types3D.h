#ifndef __TYPES3D_H_
#define __TYPES3D_H_

#include "MyriadConfig.h"
#include "core.h"

namespace Myriad
{
    class MYR_API Vector3
    {
      public:
        float x;
        float y;
        float z;
    };

    class MYR_API Vector4
    {
      public:
        float x;
        float y;
        float z;
        float w;
    };

    class MYR_API Quaternion
    {
      public:
        float x;
        float y;
        float z;
        float w;
    };
} // namespace Myriad
#endif