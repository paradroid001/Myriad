#include <catch2/catch_test_macros.hpp>
#include "core/memory/AllocatorQD.h"
#include "core/MyrHandle.h"
#include "io/Log.h"

unsigned int Factorial(unsigned int number)
{
  return number <= 1 ? number : Factorial(number - 1) * number;
}

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
  Myriad::AllocatorProvider *allocator = new Myriad::AllocatorQD();
  allocator->Init();
  uint16_t id = allocator->Alloc<int>();
  *(allocator->Get<int>(id)) = 5;
  REQUIRE(allocator != NULL);
  REQUIRE(*(allocator->Get<int>(id)) == 5);
}
