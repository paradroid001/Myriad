#ifndef TEST_TESTGAMEOBJECT_H
#define TEST_TESTGAMEOBJECT_H

#include "myriad.h"

class TestGameObject : public Myriad::GameObject
{
private:
  Myriad::Texture2D *tex;

public:
  TestGameObject(Myriad::Allocator *allocator) : Myriad::GameObject()
  {
    tex = new Myriad::Texture2D(allocator);
    tex->Load("res/carrot.png");
  }
  ~TestGameObject()
  {
    tex->Unload();
    delete tex;
    MYR_TRACE("Destructed TestGameObject");
  }
  virtual void Update(float dt) override;
  virtual void Draw(Myriad::Renderer &renderer) override;
};

void TestGameObject::Update(float dt)
{
  Myriad::Vector2 screendims = Myriad::MyrAppData::Instance()->GetScreenDimensions();
  float WIDTH = screendims.x;
  float HEIGHT = screendims.y;
  x += v_x * dt;
  y += v_y * dt;
  if (x > WIDTH || x < 0)
  {
    if (x > WIDTH)
      x = WIDTH;
    if (x < 0)
      x = 0;
    v_x = -v_x;
  }
  if (y > HEIGHT || y < 0)
  {
    if (y > HEIGHT)
      y = HEIGHT;
    if (y < 0)
      y = 0;
    v_y = -v_y;
  }
}

void TestGameObject::Draw(Myriad::Renderer &renderer)
{
  // BeginDrawing();
  // ClearBackground({0, 0, 0, 255});
  // renderer.DrawCircle({x, y}, 5.0f, {255, 0, 0, 255});
  renderer.DrawTexture(*tex, {x, y}, {255, 255, 255, 255});
  // EndDrawing();
}

#endif
