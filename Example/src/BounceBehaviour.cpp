#include "BounceBehaviour.h"

using namespace Myriad;

void BounceBehaviour::Init(float speed, Vector2 screendimensions)
{
  movespeed_ = speed;
  screensize_ = screendimensions;
  current_movement_ = {MyrRandom::Float(-1.0f, 1.0f), MyrRandom::Float(-1.0f, 1.0f)};
  p_transform = GetOwner()->GetComponent<Transform>();
  SetUpdateable(true);
  SetRenderable(false);
}

void BounceBehaviour::Update(float dt)
{
  Vector3 pos = p_transform->GetPosition();
  Vector3 movement = {movespeed_ * current_movement_.x * dt, movespeed_ * current_movement_.y * dt};
  if (pos.x + movement.x > screensize_.x || pos.x + movement.x < 0)
  {
    current_movement_.x = -current_movement_.x;
  }

  if (pos.y + movement.y > screensize_.y || pos.y + movement.y < 0)
  {
    current_movement_.y = -current_movement_.y;
  }

  pos.x += (current_movement_.x * movespeed_ * dt);
  pos.y += (current_movement_.y * movespeed_ * dt);

  p_transform->SetPosition(pos.x, pos.y, 0);
}
