#ifndef __CAMERA_H_
#define __CAMERA_H_
// #include "GameObject.h"
#include "core.h"
#include "raylib.h" //for the color typedef
#include <list>
namespace myriad
{
    class GameObject;
    typedef Color MyrColour;

    class MYR_API Camera
    {
      public:
        Camera();
        ~Camera();
        void SetBackgroundColour(MyrColour c);
        void Draw(std::list<myriad::GameObject> *drawlist);

      private:
        MyrColour backgroundColour;
    };
} // namespace myriad
#endif