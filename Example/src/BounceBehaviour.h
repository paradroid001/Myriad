#ifndef BOUNCEBEHAVIOUR_H
#define BOUNCEBEHAVIOUR_H

#include "myriad.h"

using namespace Myriad;
using namespace Myriad::ObjectComponent;

class BounceBehaviour : public Component<BounceBehaviour>
{
protected:
  float movespeed_;
  Vector2 screensize_;
  Vector2 current_movement_;
  Transform *p_transform;

public:
  void Init(float speed, Vector2 screendimensions);
  void Update(float dt);
};
#endif
