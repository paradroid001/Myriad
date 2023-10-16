#ifndef __CAMERA_H_
#define __CAMERA_H_

#include <list>
// don't #include "MyriadConfig.h" for now
#include "core.h"
// #include "raylib.h" //for the color typedef

namespace Myriad
{
    class GameObject;

    class MYR_API Camera
    {
      public:
        Camera();
        ~Camera();
        void SetBackgroundColour(Myriad::MyrColour c);
        void Draw(std::list<Myriad::GameObject *> *drawlist);

      private:
        Myriad::MyrColour backgroundColour;
    };
} // namespace Myriad
#endif