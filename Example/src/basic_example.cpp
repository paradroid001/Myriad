#include "myriad.h"
#include <iostream>
#include "basic_logger.h"
#include <cstring>

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

public:
  TestGameObjectBase()
  {
    data = new TestGameObjectBaseData();
    data->started = false;
  }
  ~TestGameObjectBase()
  {
    Myriad::MyrGameEngine::Engine()->GetAssetManager().ReleaseTexture(data->texid);
  }
  void Start()
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
  void Update(float dt)
  {
    data->pos.x += data->movement.x * data->movespeed * dt;
    data->pos.y += data->movement.y * data->movespeed * dt;
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

class MyriadExample : public Myriad::MyrGameApplication
{

private:
  MYR_ID_t texture1_id;
  MYR_ID_t font1_id;
  std::vector<TestGameObjectBase *> v_gameobjects;
  std::stringstream stats_string_;

public:
  virtual ~MyriadExample() {}

  float DoWork()
  {
    PROFILE_SCOPED
    float sum = 0.0f;
    for (int i = 0; i < 500; i++)
    {
      sum += sqrt(i);
    }
    PROFILE_LOG("Log sum");
    return sum;
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
    return config;
  }

  virtual void Start() override
  {
    texture1_id = engine_.GetAssetManager().GetTexture("res/carrot.png");
    font1_id = engine_.GetAssetManager().GetFont("res/dejavu.fnt");

    int num_objects = 5000;
    for (int i = 0; i < num_objects; i++)
    {
      v_gameobjects.push_back(new TestGameObjectBase());
    }
    return;
    /*
    MYR_TRACE("Hello, world! {0} {1} {2}", "args:", 7, 14.78);
    MYR_INFO("Hello, world! {0} {1} {2}", "args:", 7, 14.78);
    MYR_WARN("Hello, world! {0} {1} {2}", "args:", 7, 14.78);
    MYR_ERROR("Hello, world! {0} {1} {2}", "args:", 7, 14.78);

    Myriad::MyrEntity *p_e = Myriad::MyrEntityManager::CreateEntity<Myriad::MyrEntity>();
    MYR_TRACE("Entity id is: {0}", p_e->GetID());
    p_e = Myriad::MyrEntityManager::CreateEntity<Myriad::MyrEntity>();
    MYR_TRACE("Entity id is: {0}", p_e->GetID());
    p_e = Myriad::MyrEntityManager::CreateEntity<Myriad::MyrEntity>();
    MYR_TRACE("Entity id is: {0}", p_e->GetID());
    p_e = Myriad::MyrEntityManager::CreateEntity<Myriad::MyrEntity>();
    MYR_TRACE("Entity id is: {0}", p_e->GetID());
    p_e = Myriad::MyrEntityManager::CreateEntity<Myriad::MyrEntity>();
    MYR_TRACE("Entity id is: {0}", p_e->GetID());

    Myriad::MyrComponent c;
    MYR_TRACE("Created component id is {0}", c.GetID());
    MYR_TRACE("Created component entity id is {0}", c.GetEID());

    Myriad::MyrEntity *e0 = Myriad::MyrEntityManager::GetEntity(0);
    Myriad::MyrEntity *e1 = Myriad::MyrEntityManager::GetEntity(1);
    Myriad::MyrEntityManager::AddComponent(*e1, c);

    MYR_TRACE("Attached component {0} to entity {1}, component eid is now {2}", c.GetID(), e1->GetID(), c.GetEID());

    Myriad::Transform *t1 = Myriad::MyrEntityManager::CreateComponent<Myriad::Transform>();
    MYR_TRACE("Transform data is x: {0}, y: {1}, z: {2}", t1->GetPosition().x, t1->GetPosition().y, t1->GetPosition().z);
    t1->SetPosition(1, 2, 3);
    MYR_TRACE("Transform data is x: {0}, y: {1}, z: {2}", t1->GetPosition().x, t1->GetPosition().y, t1->GetPosition().z);

    Myriad::Transform *t2 = Myriad::MyrEntityManager::CreateComponent<Myriad::Transform>();
    MYR_TRACE("Transform data is x: {0}, y: {1}, z: {2}", t2->GetPosition().x, t2->GetPosition().y, t2->GetPosition().z);
    t2->SetPosition(4, 5, 6);
    MYR_TRACE("Transform data is x: {0}, y: {1}, z: {2}", t2->GetPosition().x, t2->GetPosition().y, t2->GetPosition().z);

    MyComponent *mycomp1 = Myriad::MyrEntityManager::CreateComponent<MyComponent>();
    mycomp1->SetName("Brad");
    MYR_TRACE("Component of type {0} has name {1}", static_cast<int>(mycomp1->GetType()), mycomp1->GetName());
    mycomp1->SetName("Nat");
    MyComponent *mycomp2 = Myriad::MyrEntityManager::CreateComponent<MyComponent>();
    mycomp2->SetName("Emily");
    MYR_TRACE("Component of type {0} has name {1}", static_cast<int>(mycomp2->GetType()), mycomp2->GetName());
    MYR_TRACE("Component of type {0} has name {1}", static_cast<int>(mycomp1->GetType()), mycomp1->GetName());

    // So this gets deleted first.
    Myriad::MyrComponent *c2 = Myriad::MyrEntityManager::CreateComponent<Myriad::MyrComponent>();
    // Myriad::GameObject *g = new Myriad::GameObject();
    */
  }

  void Update() override
  {
    for (auto p : v_gameobjects)
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
  }

  void Render() override
  {
    engine_.GetRenderer().BeginDrawing();
    engine_.GetRenderer().ClearBackground({127, 127, 127, 255});

    for (auto p : v_gameobjects)
    {
      if (p != nullptr && p->IsStarted() && p->IsAlive())
      {
        p->Render(engine_.GetRenderer());
      }
    }

    stats_string_.str("");
    stats_string_ << "Frame Time: " << GetFrameElapsedMS() << "ms";

    engine_.GetRenderer().DrawTexture(texture1_id, {100, 100}, {255, 255, 255, 255});
    engine_.GetRenderer().DrawText(font1_id, stats_string_.str(), {20, 20}, 25, {255, 255, 255, 255});
    engine_.GetRenderer().EndDrawing();
  }

  void PreShutdown() override
  {
    for (auto p : v_gameobjects)
    {
      if (p != nullptr)
      {
        delete p;
        p = nullptr;
        MYR_TRACE("Preshutdown Deleted object");
      }
    }
  }
};

Myriad::MyrApplication *Myriad::CreateApplication()
{
  return new MyriadExample();
}
