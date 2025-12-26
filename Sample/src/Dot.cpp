#include "Dot.h"
// #include "core/Camera.h"
// #include "core/Log.h"
// #include "myriad.h"
// #include "raylib.h"
#include <cstdlib> //ramd

Dot::Dot(flecs::world &w) : GameObject()
{
    this->e = w.entity();
    // MYR_WARN("Dot Constructor");
    //  Initial movement
    movement.x = rand() % 6 - 3;
    movement.y = rand() % 6 - 3;

    this->e.add<Velocity>();
    this->e.set<Velocity>({movement.x, movement.y});
    this->e.add<Position>();

    if (movement.x < 0)
    {
        colour = (Myriad::MyrColour){253, 249, 0, 255};
    }
    if (movement.x > 0)
    {
        colour = (Myriad::MyrColour){230, 41, 55, 255};
    }
}
Dot::~Dot() { MYR_WARN("Dot Destructor"); }

void Dot::Update()
{
    // MYR_INFO("Dot::Update...");
    const Myriad::Vector3 v = this->_ptransform->position();
    Myriad::Vector3 newpos;
    newpos.x = v.x + movement.x;
    newpos.y = v.y + movement.y;

    if (newpos.x > 800 || newpos.x < 0)
    {
        movement.x = -movement.x;
    }

    if (newpos.y > 600 || newpos.y < 0)
    {
        movement.y = -movement.y;
    }

    this->_ptransform->SetPosition(v.x + movement.x, v.y + movement.y, 0);
}

void Dot::Draw()
{
    DrawCircleShape(_ptransform->position().x, _ptransform->position().y, 2,
                    colour);
}