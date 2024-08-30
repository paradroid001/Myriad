#include <catch2/catch_test_macros.hpp>
// if you include this, you have to implement CreateApplication
// #include "myriad.h"

#include "core/memory/Allocator.h"
#include "core/memory/MyrHandle.h"
#include "core/event/MyrEvent.h"
#include "core/event/MyrEventService.h"
#include "core/MyrTimer.h"
#include "core/object/GameObject.h"
#include "core/component/MyrComponent.h"
#include <iostream>

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
  Myriad::MyrEventService::GetInstance().StartService();
  Myriad::MyrEvent::SetEventService(&(Myriad::MyrEventService::GetInstance()));

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
  Myriad::MyrEventService::GetInstance().ProcessEvents();
  Myriad::MyrEventService::GetInstance().StopService();
}

class TestGameObject : public Myriad::GameObject
{
public:
  TestGameObject(std::string name) : Myriad::GameObject(name) {}
};

class TestComponent : public Myriad::MyrComponent
{
public:
  bool ReleaseComponent() override
  {
    return true;
  }
};

TEST_CASE("Make gameobjects and add / remove children")
{

  TestGameObject *tparent = new TestGameObject("Parent");
  TestGameObject *tchild0 = new TestGameObject("Child0");
  TestGameObject *tchild1 = new TestGameObject("Child1");
  TestGameObject *tchild2 = new TestGameObject("Child2");
  TestGameObject *tchild3 = new TestGameObject("Child3");

  REQUIRE(tparent->NumChildren() == 0);
  REQUIRE(tparent->AddChild(tchild0) == true);
  REQUIRE(tparent->NumChildren() == 1);
  REQUIRE(tparent->AddChild(tchild1) == true);
  REQUIRE(tparent->NumChildren() == 2);
  REQUIRE(tparent->AddChild(tchild2) == true);
  REQUIRE(tparent->NumChildren() == 3);

  REQUIRE(tparent->GetChildIndex(tchild0) == 0);
  REQUIRE(tparent->GetChildIndex(tchild1) == 1);
  REQUIRE(tparent->GetChildIndex(tchild2) == 2);

  // Try to add child 2 again
  REQUIRE(tparent->AddChild(tchild2) == false);
  // Make sure there are still only 3 elements
  REQUIRE(tparent->NumChildren() == 3);
  // Remove child 1
  REQUIRE(tparent->GetChild(1) == tchild1);                    // Check that child1 is at index 1
  REQUIRE(tparent->RemoveChild(tparent->GetChild(1)) == true); // remove at index 1
  // Make sure I can't remove it again
  REQUIRE(tparent->RemoveChild(tchild1) == false);
  // Child 1 should be gone now.
  // So it should look like:
  // parent
  //  child0
  //  child2
  REQUIRE(tparent->NumChildren() == 2);
  REQUIRE(tparent->GetChildIndex(tchild0) == 0);
  REQUIRE(tparent->GetChildIndex(tchild2) == 1);
  REQUIRE(tparent->GetChildIndex(tchild1) == Myriad::INVALID_CHILD_INDEX);
}

TEST_CASE("Make gameobjects and add components")
{
  TestGameObject *tgo = new TestGameObject("New Object");
  TestComponent *tc = new TestComponent();
  // tc should have no owner
  REQUIRE(tc->GetOwner() == nullptr);
  tgo->AddComponent(tc);
  // tc owner should be tgo.
  REQUIRE(tc->GetOwner() == tgo);
  REQUIRE(tgo->GetComponentCount() == 2); // there is transform also?
  REQUIRE(tgo->GetComponentTypeCount() == 2);
  Myriad::Transform *t = tgo->GetComponent<Myriad::Transform>();
  TestComponent *c = tgo->GetComponent<TestComponent>();
  Myriad::MyrComponent *m = tgo->GetComponent<Myriad::MyrComponent>();
  REQUIRE(t == &(tgo->GetTransform()));
  REQUIRE(c == tc);
  REQUIRE(m == nullptr);
}
