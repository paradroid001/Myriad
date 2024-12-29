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

// An updater component for the player
class PlayerUpdaterComponent : public Myriad::MyrComponentBase<PlayerUpdaterComponent>, public Myriad::IUpdateable
{
public:
  Myriad::Vector2 *p_owner_input;
  float speed = 50.0f;

  virtual void Update(float dt = 0) override
  {
    if (p_owner_input != nullptr)
    {
      Myriad::Vector3 pos = static_cast<Myriad::GameObject *>(owner_)->GetTransform().GetPosition();
      // get the owner's input, and use that to move.
      pos.x += p_owner_input->x * speed * dt;
      pos.y += p_owner_input->y * speed * dt;
      static_cast<Myriad::GameObject *>(owner_)->GetTransform().SetPosition(pos);
    }
  }
};

class PlayerGameObject : public Myriad::GameObject
{
private:
  Myriad::Texture2D *tex;
  Myriad::Font *font;
  Myriad::Vector2 input_;
  float speed_ = 150.0f;

public:
  PlayerGameObject(Myriad::Allocator *allocator, Myriad::MyrObjectManager *p_mgr) : Myriad::GameObject(p_mgr)
  {
    tex = new Myriad::Texture2D(allocator);
    tex->Load("res/carrot.png");
    font = new Myriad::Font(allocator);
    font->Load("res/dejavu.fnt");

    input_.x = 0;
    input_.y = 0;

    // drawer_ = AddComponent<Myriad::SpriteRenderer>(Myriad::MyrApplication::GetEngine()->GetAssetManager(), "res/carrot.png");
    drawer_ = AddComponent<Myriad::TextRenderer>(Myriad::MyrApplication::GetEngine()->GetAssetManager(), "PLAYER", "res/dejavu.fnt", 10);
    updater_ = AddComponent<PlayerUpdaterComponent>();
    // TODO this is really doddy.
    static_cast<PlayerUpdaterComponent *>(updater_)->p_owner_input = &input_;
    static_cast<PlayerUpdaterComponent *>(updater_)->speed = speed_;

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
    // MYR_INFO("OnInput Called");
    InputAxisEvent *p_axis_event = static_cast<InputAxisEvent *>(p_event);
    input_.x = p_axis_event->GetAxisX();
    input_.y = p_axis_event->GetAxisY();
  }

  Myriad::Vector2 GetInput()
  {
    return input_;
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
