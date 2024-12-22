#ifndef TEST_TESTGAMEOBJECT_H
#define TEST_TESTGAMEOBJECT_H

#include "myriad.h"

class TestGameObjectUpdater : public Myriad::MyrComponentBase<TestGameObjectUpdater>, public Myriad::IUpdateable
{
private:
  Myriad::Vector2 velocity_;

public:
  // MyrComponent
  bool InitComponent(Myriad::MyrObject *owner) override
  {
    Myriad::MyrComponent::InitComponent(owner);
    return true;
  }
  Myriad::Vector2 &GetVelocity()
  {
    return velocity_;
  }
  void SetVelocity(float x, float y)
  {
    velocity_.x = x;
    velocity_.y = y;
  }
  bool ReleaseComponent() override
  {
    return true;
  }
  // IUpdateable
  void Update(float dt);
};

void TestGameObjectUpdater::Update(float dt)
{
  Myriad::Vector2 screendims = Myriad::MyrApplication::GetAppData()->GetScreenDimensions();
  float WIDTH = screendims.x;
  float HEIGHT = screendims.y;
  Myriad::Vector3 pos = static_cast<Myriad::GameObject *>(owner_)->GetTransform().GetPosition();
  pos.x += velocity_.x * dt;
  pos.y += velocity_.y * dt;
  if (pos.x > WIDTH || pos.x < 0)
  {
    if (pos.x > WIDTH)
      pos.x = WIDTH;
    if (pos.x < 0)
      pos.x = 0;
    velocity_.x = -velocity_.x;
  }
  if (pos.y > HEIGHT || pos.y < 0)
  {
    if (pos.y > HEIGHT)
      pos.y = HEIGHT;
    if (pos.y < 0)
      pos.y = 0;
    velocity_.y = -velocity_.y;
  }
  static_cast<Myriad::GameObject *>(owner_)->GetTransform().SetPosition(pos);
}

class TestGameObject : public Myriad::GameObject
{
  // private:
  //    Myriad::Texture2D *tex;
  // TestGameObjectUpdater *p_test_object_updater_;
  // Myriad::SpriteRenderer *p_sprite_renderer_;

public:
  TestGameObject(Myriad::Allocator *allocator, Myriad::AssetManager *p_asset_manager, Myriad::MyrObjectManager *p_mgr) : Myriad::GameObject(p_mgr)
  {
    // tex = new Myriad::Texture2D(allocator);
    // tex->Load("res/carrot.png");
    updater_ = AddComponent<TestGameObjectUpdater>();
    drawer_ = AddComponent<Myriad::SpriteRenderer>(p_asset_manager, "res/carrot.png");
    // Myriad::SpriteRenderer *p_sr = new Myriad::SpriteRenderer(allocator, "res/carrot.png");
    // drawer_ = p_sr;
    // AddComponent(p_sr);
  }
  ~TestGameObject()
  {
    MYR_TRACE("Destructed TestGameObject");
  }

  // Myriad::SpriteRenderer &GetSpriteRenderer() { return *p_sprite_renderer_; }
  // TestGameObjectUpdater &GetUpdater() { return *p_test_object_updater_; }
  //  virtual void Update(float dt) override;
  //  virtual void Draw(Myriad::Renderer &renderer) override;
};

// void TestGameObject::Draw(Myriad::Renderer &renderer)
//{
//   renderer.DrawTexture(*tex, {x, y}, {255, 255, 255, 255});
// }

#endif
