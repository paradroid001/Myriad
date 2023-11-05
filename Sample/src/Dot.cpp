#include "Dot.h"
#include "core/Transform.h"
#include "myriad.h"
#include <cstdlib> //ramd
#include <iostream>

Dot::Dot() : Myriad::GameObject()
{
    // MYR_WARN("Dot Constructor");
    //  Initial movement
    movement.x = rand() % 6 - 3;
    movement.y = rand() % 6 - 3;

    // this->e.add<Velocity>();
    // this->e.set<Velocity>({movement.x, movement.y});
    // this->e.add<Position>();

    if (movement.x < 0)
    {
        colour = (Myriad::MyrColour){253, 249, 0, 255};
    }
    if (movement.x > 0)
    {
        colour = (Myriad::MyrColour){230, 41, 55, 255};
    }

    Myriad::Renderer *r = new Myriad::Renderer();
    AddComponent<Myriad::RendererData>(r, r->Data());
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

    Myriad::TransformData td = {
        true, this->_ptransform, this->_ptransform->position(),
        this->_ptransform->rotation(), this->_ptransform->scale()};

    // try to force the entities to move?
    this->Entity().set<Myriad::TransformData>(td);
    // std::cout << "Moved" << std::endl;
}

void Dot::Draw()
{
    // DrawCircleShape(_ptransform->position().x, _ptransform->position().y, 2,
    //                 colour);
}