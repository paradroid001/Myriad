#include "myriad_ext.h"

#include <iostream>
#include "basic_logger.h"
#include <cstring>

#include "TestGameObjectBase.h"
#include "TestGamePlayer.h"
#include "TestGameObjectManager.h"
#include "BounceBehaviour.h"
#include "TestMovementBehaviour.h"

class MyEvent : public Myriad::MyrEvent
{
public:
  MyEvent(MYREVENT_PRIMARYTYPE_t t, MYREVENT_SUBTYPE_t s) : MyrEvent(t, s) {}
  int num_data;
  char char_data[20];
};

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
    Screen.width = 800.0f;
    Screen.height = 600.0f;
    Myriad::EngineConfig_t config;
    config.window_title = "Test";
    config.screen_dimensions = {Screen.width, Screen.height};
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

    // Sign up for events.
    MyEvent::Register<MyriadExample>(1, 1, this, &MyriadExample::TestProcessMyEvent);

    p_component_manager_ = &(engine_.GetComponentManager());
    p_gameobject_manager_ = &(engine_.GetGameObjectManager());
    font1_id = engine_.GetAssetManager().GetFont("res/dejavu.fnt");

    // I think either go manager has limit 1000
    // or
    // component manager has limit 3000
    int num_objects = 900;
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
      go_id = CreateObject();
      // go_id = p_gameobject_manager_->CreateObject<GameObject>();
      // p_go = p_gameobject_manager_->GetObject(go_id);
      p_go = GetObject(go_id);
      Vector3 pos = {MyrRandom::Float(0, Screen.width), MyrRandom::Float(0, Screen.height), 0.0f};
      p_go->AddComponent<Transform>(pos);
      p_go->AddComponent<SpriteRenderer>(&(engine_.GetAssetManager()), path);
      p_go->AddComponent<BounceBehaviour>(100.0f, Vector2(Screen.width, Screen.height));
    }

    // Now make a 'player'
    std::string player_sprite_path = "res/Druid.png";
    // MYR_ID_t player_id = p_gameobject_manager_->CreateObject<GameObject>();
    MYR_ID_t player_id = CreateObject();
    // GameObject *p_player = p_gameobject_manager_->GetObject(player_id);
    GameObject *p_player = GetObject(player_id);
    Vector3 pos = {200, 200, 0};
    p_player->AddComponent<Transform>(pos);
    p_player->AddComponent<SpriteRenderer>(&(engine_.GetAssetManager()), player_sprite_path);
    p_player->AddComponent<TestMovementBehaviour>(200.0f, Vector2(Screen.width, Screen.height));

    MyEvent *e = new MyEvent(1, 1); // type and subtype
    strncpy(e->char_data, "Creation\0", 10);
    e->num_data = player_id;
    e->Emit();

    return; //<-- why is this line here?
  }

  void TestProcessMyEvent(Myriad::MyrEvent *p_event)
  {
    // We have to cast. This sucks.
    MyEvent *p_myevent = static_cast<MyEvent *>(p_event);
    MYR_INFO("Processed: {0}: {1}", p_myevent->char_data, p_myevent->num_data);
  }

  void Update() override
  {
    for (auto go : *p_gameobject_manager_)
    {
      go->Update(1.0f / 60.0f);
    }

    // Process Events
    Myriad::MyrGameEngine::Engine()->GetEventService().ProcessEvents();
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

    // Just draw a carrot.
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
