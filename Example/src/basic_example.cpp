#include "myriad.h"
#include <iostream>
#include "basic_logger.h"
#include <cstring>

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
    return {
        {800.0f, 600.0f}, "Test", false, 0};
  }

  virtual void Start() override
  {
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

  void Render() override
  {
    engine_.GetRenderer().BeginDrawing();
    engine_.GetRenderer().ClearBackground({127, 127, 127, 255});
    engine_.GetRenderer().EndDrawing();
  }
};

Myriad::MyrApplication *Myriad::CreateApplication()
{
  return new MyriadExample();
}
