#include "myriad_ext.h"

#include <iostream>
#include "basic_logger.h"
#include <cstring>

#include "TestGameObjectBase.h"
#include "TestGamePlayer.h"
#include "TestGameObjectManager.h"
#include "BounceBehaviour.h"

class MyComponent : public Myriad::MyrComponent
{
protected:
  int level;
  float health;
  char name[10];

public:
  MyComponent() : Myriad::MyrComponent()
  {
    MYR_TRACE("Constructor for MyComponent");
    type_ = static_cast<Myriad::MyrComponentType>(12); // arbitrary type
  }
  virtual ~MyComponent()
  {
    MYR_TRACE("MyComponent destructor");
  }
  inline int GetLevel() const { return level; }
  void SetLevel(int newlevel) { level = newlevel; }
  inline float GetHealth() const { return health; }
  inline const char *GetName() const { return &name[0]; }
  void SetName(const char *newname)
  {
    strncpy(name, newname, 9);
    name[9] = '\0';
  }
};

using namespace Myriad;
using namespace Myriad::ObjectComponent;

class MyriadExample : public MyrGameApplication
{

private:
  MYR_ID_t texture1_id;
  MYR_ID_t font1_id;
  std::stringstream stats_string_;
  TestGameObjectManager object_manager_;

  GameObjectManager *p_gameobject_manager_;
  ComponentManager *p_component_manager_;

public:
  virtual ~MyriadExample()
  {
  }

  Myriad::EngineConfig_t Setup() override
  {
    Myriad::EngineConfig_t config;
    config.window_title = "Test";
    config.screen_dimensions = {800, 600};
    config.threads_enabled = false;
    config.num_threads = 0;
    config.fps = 60;
    config.asset_root_path = "res";
    config.max_component_slots = 100000;
    config.max_gameobject_slots = 1000;
    return config;
  }

  virtual void Start() override
  {
    p_component_manager_ = &(engine_.GetComponentManager());
    p_gameobject_manager_ = &(engine_.GetGameObjectManager());
    font1_id = engine_.GetAssetManager().GetFont("res/dejavu.fnt");

    int num_objects = 999;
    /*
    for (int i = 0; i < num_objects; i++)
    {
      object_manager_.CreateObject<TestGameObjectBase>();
    }
    object_manager_.CreateObject<TestGamePlayer>();
    */

    MYR_ID_t go_id;
    std::string path = "res/carrot.png";
    GameObject *p_go;
    for (int i = 0; i < num_objects; i++)
    {
      go_id = p_gameobject_manager_->CreateObject<GameObject>();
      p_go = p_gameobject_manager_->GetObject(go_id);
      Vector3 pos = {MyrRandom::Float(0, 800.0f), MyrRandom::Float(0, 600.0f), 0.0f};
      p_go->AddComponent<Transform>(pos);
      p_go->AddComponent<SpriteRenderer>(&(engine_.GetAssetManager()), path);
      p_go->AddComponent<BounceBehaviour>(100.0f, Vector2(800.0f, 600.0f));
    }

    return;
  }

  void Update() override
  {
    /*
    for (auto p : object_manager_)
    {
      if (p != nullptr)
      {
        if (!p->IsStarted())
        {
          p->Start();
        }
        else
        {
          p->Update(1.0f / 60.f);
        }
      }
    }
    */

    for (auto go : *p_gameobject_manager_)
    {
      go->Update(1.0f / 60.0f);
    }
  }

  void Render() override
  {
    Myriad::Renderer &r = engine_.GetRenderer();
    r.BeginDrawing();
    r.ClearBackground({127, 127, 127, 255});

    /*
    for (auto p : object_manager_)
    {
      if (p != nullptr && p->IsStarted() && p->IsAlive())
      {
        p->Render(r);
      }
    }
    */
    for (auto go : *p_gameobject_manager_)
    {
      go->Render(r);
    }

    stats_string_.str("");
    stats_string_ << "Frame Time: " << GetFrameElapsedMS() << "ms";

    r.DrawTexture(texture1_id, {100, 100}, {255, 255, 255, 255});
    r.DrawText(font1_id, stats_string_.str(), {20, 20}, 25, {255, 255, 255, 255});
    r.EndDrawing();
  }

  void PreShutdown() override
  {
  }
};

Myriad::MyrApplication *Myriad::CreateApplication()
{
  return new MyriadExample();
}
