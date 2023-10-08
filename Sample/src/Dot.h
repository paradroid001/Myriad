#ifndef __DOT_H_
#define __DOT_H_

// #include "core/Camera.h" //myrcolour
// #include <core/GameObject.h>
#include "myriad.h"

class Dot : public Myriad::GameObject
{
  public:
    Dot();
    ~Dot();
    void Draw() override;
    void Update() override;

  private:
    Myriad::Vector3 movement;
    Myriad::MyrColour colour;
};

#endif