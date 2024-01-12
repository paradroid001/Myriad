#ifndef __CAMERA_H_
#define __CAMERA_H_

#include <list>

// Not including flecs for now
// #include "flecs.h"

// don't #include "MyriadConfig.h" for now
#include "core/GameObject.h"
#include "core/core.h"
#include "rendering/RendererGroup.h"

namespace Myriad
{
    // class GameObject;

    class MYR_API Camera
    {
      public:
        Camera();
        ~Camera();
        void SetBackgroundColour(Myriad::MyrColour c);
        void Draw(RendererGroup &renderers);

      private:
        Myriad::MyrColour backgroundColour;
        // flecs::system drawSystem;
    };
} // namespace Myriad
#endif