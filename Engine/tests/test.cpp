#include <catch2/catch_test_macros.hpp>
// if you include this, you have to implement CreateApplication
// #include "myriad.h"

#include "core/memory/Allocator.h"
#include "core/memory/MyrHandle.h"

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
