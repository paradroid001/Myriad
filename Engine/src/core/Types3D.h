#ifndef __TYPES3D_H_
#define __TYPES3D_H_

// #ifdef MYRIAD_INTERNAL
#include "MyriadConfig.h"
#include "core.h"

// #include <raylib.h>
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