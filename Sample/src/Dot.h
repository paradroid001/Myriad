#ifndef __DOT_H_
#define __DOT_H_

// #include "core/Camera.h" //myrcolour
// #include <core/GameObject.h>
#include "main.h"

class Dot : public Myriad::GameObject
{
  public:
    Dot(flecs::world &w);
    ~Dot();
    void Draw() override;
    void Update() override;
    flecs::entity &Entity() { return e; };

  private:
    Myriad::Vector3 movement;
    Myriad::MyrColour colour;
    flecs::entity e;
};

#endif