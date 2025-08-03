#pragma once
#include "myriad.h"
#include "TestGameObjectBase.h"

class TestGamePlayer : public TestGameObjectBase
{
public:
  virtual void Start()
  {
    data->texid = Myriad::MyrGameEngine::Engine()->GetAssetManager().GetTexture("res/Druid.png");
    data->pos.x = 200;
    data->pos.y = 200;
    data->movement.x = 0;
    data->movement.y = 0;
    data->started = true;
    data->alive = true;
    data->movespeed = 100;
  }

  virtual void Update(float dt)
  {
    const int KEYCODE_A = 65;
    const int KEYCODE_W = 87;
    const int KEYCODE_D = 68;
    const int KEYCODE_S = 83;

    Myriad::Vector2 mov = {0, 0};
    // Myriad::KeyboardInput &kb = Myriad::MyrGameEngine::Engine()->Input();
    /*
    if (kb.IsKeyDown(KEYCODE_A))
      mov.x -= 1;
    if (kb.IsKeyDown(KEYCODE_D))
      mov.x += 1;
    if (kb.IsKeyDown(KEYCODE_W))
      mov.y -= 1;
    if (kb.IsKeyDown(KEYCODE_S))
      mov.y += 1;
    */
    if (Myriad::Input.IsKeyDown(KEYCODE_A))
      mov.x -= 1;
    if (Myriad::Input.IsKeyDown(KEYCODE_D))
      mov.x += 1;
    if (Myriad::Input.IsKeyDown(KEYCODE_W))
      mov.y -= 1;
    if (Myriad::Input.IsKeyDown(KEYCODE_S))
      mov.y += 1;

    data->movement = mov;
    UpdateMovement(dt);
  }
};
