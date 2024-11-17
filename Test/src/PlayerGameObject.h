#ifndef TEST_PLAYERGAMEOBJECT_H
#define TEST_PLAYERGAMEOBJECT_H

#include "myriad.h"

const uint8_t EVENT_TYPE_KEYBOARD = 1;

// A class to represent input events on a 2D axis
class InputAxisEvent : public Myriad::MyrEvent
{
private:
  Myriad::Vector2 axis_;

public:
  InputAxisEvent(float x, float y) : Myriad::MyrEvent(Myriad::MYR_EVENT_INPUT, EVENT_TYPE_KEYBOARD), axis_({x, y}) {}
  inline float GetAxisX() const { return axis_.x; }
  inline float GetAxisY() const { return axis_.y; }
};

class PlayerGameObject : public Myriad::GameObject
{
private:
  Myriad::Texture2D *tex;
  Myriad::Vector2 input_;
  float speed_ = 50.0f;

public:
  PlayerGameObject(Myriad::Allocator *allocator, Myriad::MyrObjectManager *p_mgr) : Myriad::GameObject(p_mgr)
  {
    tex = new Myriad::Texture2D(allocator);
    tex->Load("res/carrot.png");
    input_.x = 0;
    input_.y = 0;
    InputAxisEvent::Register<PlayerGameObject>(Myriad::MYR_EVENT_INPUT, EVENT_TYPE_KEYBOARD, this, &PlayerGameObject::OnInput);
  }
  ~PlayerGameObject()
  {
    tex->Unload();
    delete tex;
    MYR_TRACE("Destructed TestGameObject");
    MYR_TRACE("Would also need to unsub from events...");
  }
  // virtual void Update(float dt) override;
  // virtual void Draw(Myriad::Renderer &renderer) override;

  void OnInput(Myriad::MyrEvent *p_event)
  {
    MYR_INFO("OnInput Called");
    InputAxisEvent *p_axis_event = static_cast<InputAxisEvent *>(p_event);
    input_.x = p_axis_event->GetAxisX();
    input_.y = p_axis_event->GetAxisY();
  }
};

/*
void PlayerGameObject::Update(float dt)
{
  x += input_.x * speed_ * dt;
  y += input_.y * speed_ * dt;
  // Reset input.
  input_.x = 0;
  input_.y = 0;
}

void PlayerGameObject::Draw(Myriad::Renderer &renderer)
{
  // BeginDrawing();
  // ClearBackground({0, 0, 0, 255});
  // renderer.DrawCircle({x, y}, 5.0f, {255, 0, 0, 255});
  renderer.DrawTexture(*tex, {x, y}, {128, 128, 255, 255});
  // EndDrawing();
}
*/
#endif
