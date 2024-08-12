#include <catch2/catch_test_macros.hpp>
// if you include this, you have to implement CreateApplication
// #include "myriad.h"

#include "core/memory/Allocator.h"
#include "core/memory/MyrHandle.h"
#include "core/event/MyrEvent.h"
#include "core/event/MyrEventService.h"
#include "core/MyrTimer.h"
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
  virtual ~TestCreature(){};
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
  MyEvent() : Myriad::MyrEvent(Myriad::MYR_EVENT_SYSTEM, 8){};
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

  MyEvent *me = new MyEvent();
  me->field1 = 5;
  me->field2 = 7.0f;
  REQUIRE(me->field1 == 5);
  REQUIRE(me->field2 == 7.0f);

  Myriad::EventDispatcher *ed = new Myriad::EventDispatcher();
  MyObject *mo = new MyObject();

  // This is the best we can do for now, and
  MyEvent::Register<MyObject>(Myriad::MYR_EVENT_SYSTEM, 8, mo, &MyObject::Run, ed);
  me->Emit(ed);
  Myriad::MyrEventService::GetInstance().StopService();
}
