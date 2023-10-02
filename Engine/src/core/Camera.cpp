#include "Camera.h"
#include "GameObject.h"
// #include "raylib.h"
#include <list>

namespace myriad
{
    using std::list;
    Camera::Camera() { this->backgroundColour = (MyrColour)BLACK; }
    Camera::~Camera() {}

    void Camera::SetBackgroundColour(MyrColour c)
    {
        this->backgroundColour = c;
    }

    void Camera::Draw(std::list<myriad::GameObject> *drawlist)
    {
        BeginDrawing();
        ClearBackground((Color)(this->backgroundColour));
        Vector2 v;
        v.x = 200;
        v.y = 200;
        DrawCircleV(v, 20, YELLOW);

        if (drawlist != NULL)
        {
            std::list<GameObject>::iterator it;
            for (it = drawlist->begin(); it != drawlist->end(); ++it)
            {
                it->Draw();
            }
        }

        EndDrawing();
    }

} // namespace myriad