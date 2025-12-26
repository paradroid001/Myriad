#include "Camera.h"
#include "GameObject.h"
#include "Types2D.h"
#include "Types3D.h"
#include <list>

namespace Myriad
{
    using std::list;
    Camera::Camera() { this->backgroundColour = (MyrColour){0, 0, 0, 255}; }

    Camera::~Camera() {}

    void Camera::SetBackgroundColour(MyrColour c)
    {
        this->backgroundColour = c;
    }

    void Camera::Draw(std::list<Myriad::GameObject *> *drawlist)
    {
        BeginDrawing();
        Color c;
        c.r = backgroundColour.r;
        c.g = backgroundColour.g;
        c.b = backgroundColour.b;
        c.a = backgroundColour.a;

        ClearBackground(c);
        Myriad::Vector2 v;
        v.x = 200;
        v.y = 200;
#if MYR_RENDERER == RAYLIB
        DrawCircleV(v, 20, YELLOW);
#endif
        if (drawlist != NULL)
        {
            std::list<GameObject *>::iterator it;
            for (it = drawlist->begin(); it != drawlist->end(); ++it)
            {
                (*it)->Draw();
            }
        }

        EndDrawing();
    }

} // namespace Myriad