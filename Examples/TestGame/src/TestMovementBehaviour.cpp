#include "TestMovementBehaviour.h"

using namespace Myriad;

void TestMovementBehaviour::Init(float speed, Vector2 screendimensions)
{
  movespeed_ = speed;
  screensize_ = screendimensions;
  current_input_ = {0, 0};
  p_transform = GetOwner()->GetComponent<Transform>();
  SetUpdateable(true);
  SetRenderable(false);
}

void TestMovementBehaviour::Update(float dt)
{
  const int KEYCODE_A = 65;
  const int KEYCODE_W = 87;
  const int KEYCODE_D = 68;
  const int KEYCODE_S = 83;
  Vector3 pos = p_transform->GetPosition();
  KeyboardInput &kbinput = MyrGameEngine::Engine()->Input();
  current_input_ = {};
  if (kbinput.IsKeyDown(KEYCODE_A))
  {
    current_input_.x -= 1;
  }
  if (kbinput.IsKeyDown(KEYCODE_D))
  {
    current_input_.x += 1;
  }
  if (kbinput.IsKeyDown(KEYCODE_W))
  {
    current_input_.y -= 1;
  }
  if (kbinput.IsKeyDown(KEYCODE_S))
  {
    current_input_.y += 1;
  }
  pos.x += (movespeed_ * current_input_.x * dt);
  pos.y += (movespeed_ * current_input_.y * dt);

  p_transform->SetPosition(pos.x, pos.y, 0);
}
