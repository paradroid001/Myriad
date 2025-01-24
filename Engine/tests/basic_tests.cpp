#include <catch2/catch_test_macros.hpp>
// #include "myriad.h" //if you include this, you have to define Myriad::CreateApplication.
#include "core/MyrEntityManager.h"
#include "core/MyrEntity.h"
#include "io/MyrLogging.h"

// For creating game things.
#include "game/GameObjectManager.h"
#include "game/GameObject.h"

unsigned int Factorial(unsigned int number)
{
  return number <= 1 ? number : Factorial(number - 1) * number;
}

// This object needs to be here so that
// myriad logging functions work.
// This is normally taken care of by MyrApplication
Myriad::MyrLogging logging;

TEST_CASE("Factorials are computed", "[factorial]")
{
  REQUIRE(Factorial(1) == 1);
  REQUIRE(Factorial(2) == 2);
  REQUIRE(Factorial(3) == 6);
  REQUIRE(Factorial(10) == 3628800);
}

TEST_CASE("Can create entities")
{
  // Setup entity manager, normally done by MyrGameApplication
  Myriad::MyrEntityManager mgr;
  Myriad::MyrEntityManager::SetEntityManager(&mgr);

  Myriad::MyrEntity *entity1 = Myriad::MyrEntityManager::CreateEntity<Myriad::MyrEntity>();
  REQUIRE(entity1 != nullptr);
  REQUIRE(entity1->GetID() < MYRIAD_MAX_ENTITIES);
  Myriad::MyrEntity *entity2 = Myriad::MyrEntityManager::GetEntity(entity1->GetID());
  REQUIRE(entity1 == entity2);
}

TEST_CASE("Can manage gameobjects")
{
  Myriad::GameObjectManager<Myriad::GameObject> gm;
  Myriad::MyrObjectID_t go1 = gm.CreateObject();
  REQUIRE(go1 == 0);
  Myriad::MyrObjectID_t go2 = gm.CreateObject();
  REQUIRE(go2 == 1);
  Myriad::MyrObjectID_t go3 = gm.CreateObject();
  REQUIRE(go3 == 2);
  Myriad::MyrGameObject *pgo1 = gm.GetObject(go1);
  REQUIRE(pgo1 != nullptr);
  REQUIRE(pgo1->GetID() == go1);
  // Destroy the object
  gm.DestroyObjectById(go1);
  pgo1 = gm.GetObject(go1);
  REQUIRE(pgo1 == nullptr);
  // Destroy it again (just to check for double deletes)
  gm.DestroyObjectById(go1);
  pgo1 = gm.GetObject(go1);
  REQUIRE(pgo1 == nullptr);

  // gm.GetObject(gm2).
}
