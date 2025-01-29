#ifndef _fdklsjkfjjdsklf_
#define _fdklsjkfjjdsklf_

#include "core/MyrAlloc.h"
#include "core/MyrObjectManager.h"
#include "util/MyrRandom.h"
#include "core/MyrGameEngine.h"
#include "core/core.h"
#include "gfx/Renderer.h"
#include "asset/AssetManager.h"

struct TestGameObjectBaseData
{
  MYR_ID_t id;
  bool started;
  bool alive;
  Myriad::TexHandle_T texid;
  Myriad::Vector2 pos;
  Myriad::Vector2 movement;
  float movespeed;
};
class TestGameObjectBase
{
protected:
  TestGameObjectBaseData *data;
  inline static MYR_ID_t s_id_pool_ = 0;
  void UpdateMovement(float dt)
  {
    data->pos.x += data->movement.x * data->movespeed * dt;
    data->pos.y += data->movement.y * data->movespeed * dt;
  }

public:
  TestGameObjectBase()
  {
    data = new TestGameObjectBaseData();
    data->id = s_id_pool_++;
    data->started = false;
  }
  virtual ~TestGameObjectBase()
  {
    Myriad::MyrGameEngine::Engine()->GetAssetManager().ReleaseTexture(data->texid);
  }
  inline MYR_ID_t GetId() const { return data->id; }
  virtual void Start()
  {
    data->texid = Myriad::MyrGameEngine::Engine()->GetAssetManager().GetTexture("res/carrot.png");
    data->pos.x = Myriad::MyrRandom::Float(0, 800);
    data->pos.y = Myriad::MyrRandom::Float(0, 600);
    data->movement.x = 1;
    data->movement.y = 1;
    data->started = true;
    data->alive = true;
    data->movespeed = 30;
  }

  virtual void Update(float dt)
  {
    UpdateMovement(dt);
    // Bounce around
    if (data->pos.x > 800 || data->pos.x < 0)
    {
      data->movement.x = -data->movement.x;
    }
    if (data->pos.y > 600 || data->pos.y < 0)
    {
      data->movement.y = -data->movement.y;
    }
  }
  void Render(Myriad::Renderer &renderer)
  {
    renderer.DrawTexture(data->texid, data->pos, {255, 255, 255, 255});
  }
  bool IsStarted()
  {
    if (data != nullptr && data->started)
      return true;
    return false;
  }
  bool IsAlive()
  {
    if (data != nullptr && data->alive)
      return true;
    return false;
  }
};

#endif
