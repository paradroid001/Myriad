#include "Camera.h"
#include "GameObject.h"
#include "Types2D.h"
#include "Types3D.h"
#include <list>

namespace Myriad
{
    using std::list;
    Camera::Camera() { this->backgroundColour = (MyrColour)BLACK; }
    Camera::~Camera() {}

    void Camera::SetBackgroundColour(MyrColour c)
    {
        this->backgroundColour = c;
    }

    void Camera::Draw(std::list<Myriad::GameObject *> *drawlist)
    {
        BeginDrawing();
        ClearBackground((Color)(this->backgroundColour));
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