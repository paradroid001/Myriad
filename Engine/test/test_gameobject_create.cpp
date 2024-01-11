// #include <catch2/catch_test_macros.hpp>
#include <catch_amalgamated.hpp>

#include "myriad.h"
#include <cstdint>

using namespace Myriad;

uint32_t factorial(uint32_t number)
{
    return number <= 1 ? number : factorial(number - 1) * number;
}

GameObject *createGameObject()
{
    GameObject *g = new GameObject();
    return g;
}

TEST_CASE("Factorials are computed", "[factorial]")
{
    REQUIRE(factorial(1) == 1);
    REQUIRE(factorial(2) == 2);
    REQUIRE(factorial(3) == 6);
    REQUIRE(factorial(10) == 3'628'800);
}

TEST_CASE("GameObject is Instantiated", "[createGameObject]")
{
    REQUIRE(createGameObject() != NULL);
}
