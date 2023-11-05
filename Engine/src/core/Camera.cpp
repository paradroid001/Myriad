#include "core/Camera.h"

#include <list>

#include "flecs.h"

#include "Entities/EntityManager.h"
#include "core/GameObject.h"
#include "core/Log.h"
#include "core/Renderer.h"
#include "core/Transform.h"
#include "core/Types2D.h"
#include "core/Types3D.h"

namespace Myriad
{
    using std::list;
    Camera::Camera()
    {
        this->backgroundColour = (MyrColour){0, 0, 0, 255};
        /*
        drawSystem =
            Myriad::Entities::EntityManager::Instance()
                ->World()
                .system<Myriad::GameObject::Data, Myriad::Renderer::Data,
                        Myriad::Transform::Data>("CameraDraw")
                .each([](Myriad::GameObject::Data &gameobject_data,
                         Myriad::Renderer::Data &renderer_data,
                         Myriad::Transform::Data &transform_data)
                      { gameobject_data.gameObject->Draw(); });
        */
        /*
        drawSystem = Myriad::Entities::EntityManager::Instance()
                         ->World()
                         .system<Myriad::GameObject::Data>("CameraDraw")
                         .each([](Myriad::GameObject::Data &gameobject_data)
                               { gameobject_data.gameObject->Draw(); });
        */

        drawSystem = Myriad::Entities::EntityManager::Instance()
                         ->World()
                         .system<Myriad::TransformData>("CameraDraw")
                         .each(
                             [](Myriad::TransformData &transform_data)
                             {
                                 // MYR_TRACE("Position: x: {0} y: {1}",
                                 //           transform_data.position.x,
                                 //           transform_data.position.y);
                                 DrawCircle(transform_data.position.x,
                                            transform_data.position.y, 2,
                                            {255, 128, 0, 255});
                             });
    }
    Camera::~Camera() {}

    void Camera::SetBackgroundColour(MyrColour c)
    {
        this->backgroundColour = c;
    }

    void Camera::Draw() // std::list<Myriad::GameObject *> *drawlist)
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
        /*
        if (drawlist != NULL)
        {
            std::list<GameObject *>::iterator it;
            for (it = drawlist->begin(); it != drawlist->end(); ++it)
            {
                (*it)->Draw();
            }
        }
        */
        drawSystem.run();

        // query for everything with a transform.

        EndDrawing();
    }

} // namespace Myriad