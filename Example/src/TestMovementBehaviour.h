#ifndef TESTMOVEMENTBEHAVIOUR_H
#define TESTMOVEMENTBEHAVIOUR_H

#include "myriad.h"

using namespace Myriad;
using namespace Myriad::ObjectComponent;

class TestMovementBehaviour : public Component<TestMovementBehaviour>
{
protected:
  float movespeed_;
  Vector2 screensize_;
  Vector2 current_input_;
  Transform *p_transform;

public:
  void Init(float speed, Vector2 screendimensions);
  void Update(float dt);
};
#endif
