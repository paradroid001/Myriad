#include <catch2/catch_test_macros.hpp>
// if you include this, you have to implement CreateApplication
// #include "myriad.h"

#include "core/memory/Allocator.h"
#include "core/memory/MyrHandle.h"
#include "core/event/MyrEvent.h"
#include "core/event/MyrEventService.h"
#include "core/MyrTimer.h"
#include "core/object/GameObject.h"
#include "core/object/MyrObjectManager.h"
#include "core/component/MyrComponent.h"

#include <iostream>
#include <string>

unsigned int Factorial(unsigned int number)
{
  return number <= 1 ? number : Factorial(number - 1) * number;
}

class TestCreature
{
protected:
  uint8_t num_legs = 0;
  uint8_t num_wings = 0;

public:
  TestCreature(uint8_t legs, uint8_t wings) : num_legs(legs), num_wings(wings) {}
  inline uint8_t GetNumLegs() { return num_legs; }
  inline uint8_t GetNumWings() { return num_wings; }
  virtual ~TestCreature() {};
};
class CreatureBat : public TestCreature
{
public:
  CreatureBat() : TestCreature(2, 2) {}
};
class CreatureDog : public TestCreature
{
public:
  CreatureDog() : TestCreature(4, 0) {}
};

TEST_CASE("Factorials are computed", "[factorial]")
{
  REQUIRE(Factorial(1) == 1);
  REQUIRE(Factorial(2) == 2);
  REQUIRE(Factorial(3) == 6);
  REQUIRE(Factorial(10) == 3628800);
}

TEST_CASE("Allocator makes handles", "[group1]")
{
  Myriad::Log::Init();
  Myriad::Allocator *allocator = new Myriad::Allocator();
  allocator->Init();
  Myriad::MyrHandle<int> h_i = allocator->Alloc<int>();
  *(h_i.Get()) = 5;
  REQUIRE(allocator != NULL);
  // REQUIRE(*(allocator->Get<int>(id)) == 5); I don't have a 'get' in allocator.
  REQUIRE(*h_i == 5);
  // h_i.Destroy(); // destroy it, should call destructor.
  // At this point, you should be able to tell that h_i is invalid.
  allocator->Shutdown();
  delete allocator;
}

TEST_CASE("Allocator correctly creates client classes", "[group1]")
{
  Myriad::Allocator *allocator = new Myriad::Allocator();
  allocator->Init();
  int num_creatures = 50;
  TestCreature *creatures[50];
  for (int i = 0; i < num_creatures / 2; i++)
  {
    Myriad::MyrHandle<CreatureBat> b = allocator->Alloc<CreatureBat>();
    creatures[i] = b.Get();
  }
  for (int i = num_creatures / 2; i < num_creatures; i++)
  {
    creatures[i] = (allocator->Alloc<CreatureBat>()).Get();
  }
  for (int i = 0; i < num_creatures; i++)
  {
    REQUIRE(creatures[i] != nullptr);
    REQUIRE(creatures[i]->GetNumLegs() == 2);
    REQUIRE(creatures[i]->GetNumWings() == 2);
  }

  // There aren't actually any tests here, I just want to know that
  // all the allocations are corectly cleaned up.
  allocator->Shutdown();
  delete allocator;
}

class MyEvent : public Myriad::MyrEvent
{
public:
  MyEvent() : Myriad::MyrEvent(Myriad::MYR_EVENT_SYSTEM, 8) {};
  int field1;
  float field2;
};

class MyObject
{
public:
  void Run(Myriad::MyrEvent *evt)
  {
    std::cout << "Hello from MyObject.Run" << std::endl;
  }
};

TEST_CASE("Can combine uint16s")
{
  Myriad::MyrEventType type = Myriad::MYR_EVENT_FRAME_LIFECYCLE;
  uint8_t subtype = 195;
  uint16_t combined = type << 8 | subtype;
  std::cout << "Combined is " << combined << " which should be " << type << " and " << subtype << std::endl;
  REQUIRE(combined > 195);
  REQUIRE(combined == 4291); // 16 * 256 + 195 = 4291
}

TEST_CASE("Timer behaves")
{
  Myriad::MyrTimer timer;
  timer.Start();
  float t1 = timer.Time();
  float t2 = timer.Time();
  timer.Stop();
  float t3 = timer.Time();
  timer.Start();
  float t4 = timer.Time();
  timer.Reset();
  timer.Stop();
  float t5 = timer.Time();
  REQUIRE(t2 > t1);
  REQUIRE(t3 == t2);
  REQUIRE(t4 > t3);
  REQUIRE(t5 == 0.0f);
}

TEST_CASE("Can Make Events")
{
  Myriad::MyrEventService *p_event_service = new Myriad::MyrEventService();
  Myriad::MyrEvent::SetEventService(p_event_service);
  p_event_service->StartService();

  MyEvent *me = new MyEvent();
  me->field1 = 5;
  me->field2 = 7.0f;
  REQUIRE(me->field1 == 5);
  REQUIRE(me->field2 == 7.0f);

  // Myriad::EventDispatcher *ed = new Myriad::EventDispatcher();
  MyObject *mo = new MyObject();

  // This is the best we can do for now, and
  MyEvent::Register<MyObject>(Myriad::MYR_EVENT_SYSTEM, 8, mo, &MyObject::Run);
  me->Emit();
  p_event_service->ProcessEvents();
  p_event_service->StopService();
  delete p_event_service;
}

class TestGameObject : public Myriad::GameObject
{
public:
  TestGameObject(std::string name, Myriad::MyrObjectManager *p_mgr) : Myriad::GameObject(name, p_mgr) {}
};

class TestComponent : public Myriad::MyrComponentBase<TestComponent>
{
public:
  bool ReleaseComponent() override
  {
    return true;
  }
};

TEST_CASE("Use a game object manager to make gameobjects and add/remove children")
{
  Myriad::MyrObjectManager *m = new Myriad::MyrObjectManager();
  m->StartService();
  Myriad::MyrHandle_T hparent = m->CreateGameObject(std::string("Parent"), m);
  Myriad::MyrHandle_T hchild1 = m->CreateGameObject(std::string("Child 1"), m);
  Myriad::MyrHandle_T hchild2 = m->CreateGameObject(std::string("Child 2"), m);
  // Myriad::ObjectNode *pparent = m.GetObject(hparent);
  // Myriad::ObjectNode *pchild = m.GetObject(hchild);
  REQUIRE(m->GetParent(hparent) == Myriad::MYRHANDLE_INVALID_INDEX);
  REQUIRE(m->GetParent(hchild1) == Myriad::MYRHANDLE_INVALID_INDEX);
  REQUIRE(m->GetNumChildren(hparent) == 0);
  REQUIRE(m->GetNumChildren(hchild1) == 0);

  // Add child to parent
  m->AddChild(hparent, hchild1);
  REQUIRE(m->GetNumChildren(hparent) == 1);
  REQUIRE(m->GetParent(hchild1) == hparent);

  m->AddChild(hparent, hchild2);
  REQUIRE(m->GetNumChildren(hparent) == 2);
  REQUIRE(m->GetParent(hchild2) == hparent);
  Myriad::GameObject *pg = m->GetGameObject(hparent);
  MYR_TRACE("Name is {0}", pg->GetName());

  m->DebugLogObjects();
  m->StopService();
  delete m;
}

TEST_CASE("Make gameobjects and add / remove children")
{
  return;
  /*
  Myriad::Allocator *go_allocator = new Myriad::Allocator();
  go_allocator->Init();
  Myriad::MyrHandle<TestGameObject> tparent = go_allocator->Alloc<TestGameObject>("Parent");
  Myriad::MyrHandle<TestGameObject> tchild0 = go_allocator->Alloc<TestGameObject>("Child0");
  Myriad::MyrHandle<TestGameObject> tchild1 = go_allocator->Alloc<TestGameObject>("Child1");
  Myriad::MyrHandle<TestGameObject> tchild2 = go_allocator->Alloc<TestGameObject>("Child2");
  Myriad::MyrHandle<TestGameObject> tchild3 = go_allocator->Alloc<TestGameObject>("Child3");

  REQUIRE(tparent.Get()->NumChildren() == 0);
  REQUIRE(tparent.Get()->AddChild(tchild0.Handle()) == true);
  REQUIRE(tparent.Get()->NumChildren() == 1);
  REQUIRE(tparent.Get()->AddChild(tchild1.Handle()) == true);
  REQUIRE(tparent.Get()->NumChildren() == 2);
  REQUIRE(tparent.Get()->AddChild(tchild2.Handle()) == true);
  REQUIRE(tparent.Get()->NumChildren() == 3);

  REQUIRE(tparent.Get()->GetChildIndex(tchild0.Handle()) == 0);
  REQUIRE(tparent.Get()->GetChildIndex(tchild1.Handle()) == 1);
  REQUIRE(tparent.Get()->GetChildIndex(tchild2.Handle()) == 2);

  // Try to add child 2 again
  REQUIRE(tparent.Get()->AddChild(tchild2.Handle()) == false);
  // Make sure there are still only 3 elements
  REQUIRE(tparent.Get()->NumChildren() == 3);
  // Remove child 1
  REQUIRE(tparent.Get()->GetChild(1) == tchild1.Handle());                 // Check that child1 is at index 1
  REQUIRE(tparent.Get()->RemoveChild(tparent.Get()->GetChild(1)) == true); // remove at index 1
  // Make sure I can't remove it again
  REQUIRE(tparent.Get()->RemoveChild(tchild1.Handle()) == false);
  // Child 1 should be gone now.
  // So it should look like:
  // parent
  //  child0
  //  child2
  REQUIRE(tparent.Get()->NumChildren() == 2);
  REQUIRE(tparent.Get()->GetChildIndex(tchild0.Handle()) == 0);
  REQUIRE(tparent.Get()->GetChildIndex(tchild2.Handle()) == 1);
  REQUIRE(tparent.Get()->GetChildIndex(tchild1.Handle()) == Myriad::INVALID_CHILD_INDEX);

  go_allocator->Shutdown();
  delete go_allocator;
  */
}

TEST_CASE("Make gameobjects and add components")
{
  Myriad::MyrObjectManager *m = new Myriad::MyrObjectManager();
  TestGameObject *tgo = new TestGameObject("New Object", m);
  TestComponent *tc = tgo->AddComponent<TestComponent>();
  // tc should have an owner
  REQUIRE(tc->GetOwner() != nullptr);
  // tc owner should be tgo.
  REQUIRE(tc->GetOwner() == tgo);
  REQUIRE(tgo->GetComponentCount() == 2); // there is transform also?
  REQUIRE(tgo->GetComponentTypeCount() == 2);
  Myriad::Transform *t = tgo->GetComponent<Myriad::Transform>();
  TestComponent *c = tgo->GetComponent<TestComponent>();
  // Myriad::MyrComponent *mc = tgo->GetComponent<Myriad::MyrComponentBase>();
  REQUIRE(t == &(tgo->GetTransform()));
  REQUIRE(c == tc);
  // REQUIRE(mc == nullptr);
  delete m;
}
