#ifndef __CAMERA_H_
#define __CAMERA_H_
#include "MyriadConfig.h"
#include "core.h"
// #include "raylib.h" //for the color typedef

#include <list>

namespace Myriad
{
    class GameObject;
    typedef Color MyrColour;

    class MYR_API Camera
    {
      public:
        Camera();
        ~Camera();
        void SetBackgroundColour(MyrColour c);
        void Draw(std::list<Myriad::GameObject *> *drawlist);

      private:
        MyrColour backgroundColour;
    };
} // namespace Myriad
#endif