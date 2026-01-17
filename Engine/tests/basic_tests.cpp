#include <catch2/catch_test_macros.hpp>
// #include "myriad.h" //if you include this, you have to define MyrApplication

/*
#include "core/MyrEntity.h"
#include "core/MyrEntityManager.h"
#include "io/MyrLogging.h"

// For creating game things.
#include "game/oc/GameObject.h"
#include "game/oc/GameObjectManager.h"
*/

#include "myriad.h"
// Deprecated?
#include "core/MyrEntity.h"
#include "core/MyrEntityManager.h"
// Needed so that there is at least some static logging pointers.
MyrLogging logging_;

unsigned int Factorial(unsigned int number)
{
    return number <= 1 ? number : Factorial(number - 1) * number;
}

// This object needs to be here so that
// myriad logging functions work.
// This is normally taken care of by MyrApplication
// Myriad::MyrLogging logging;

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

    Myriad::MyrEntity *entity1 =
        Myriad::MyrEntityManager::CreateEntity<Myriad::MyrEntity>();
    REQUIRE(entity1 != nullptr);
    REQUIRE(entity1->GetID() < MYRIAD_MAX_ENTITIES);
    Myriad::MyrEntity *entity2 =
        Myriad::MyrEntityManager::GetEntity(entity1->GetID());
    REQUIRE(entity1 == entity2);
}

using namespace Myriad::ObjectComponent;

TEST_CASE("Can manage gameobjects")
{
    GameObjectManager gm(1024); // slots
    MYR_ID_t go1 = gm.CreateObject<GameObject>();
    REQUIRE(go1 == 0);
    MYR_ID_t go2 = gm.CreateObject<GameObject>();
    REQUIRE(go2 == 1);
    MYR_ID_t go3 = gm.CreateObject<GameObject>();
    REQUIRE(go3 == 2);
    GameObject *pgo1 = gm.GetObject(go1);
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

TEST_CASE("Can use id container (map)")
{
    class TestObject
    {
      public:
        TestObject() : val(0) {}
        ~TestObject() {}
        int val = 0;
        void SetVal(int newval) { val = newval; }
        int GetVal() { return val; }
    };

    MyrIDMap<TestObject *> map;

    MYR_ID_t id = map.Add(new TestObject());
    REQUIRE(id == 1);
    TestObject **t = map.Get(id);
    REQUIRE((*t)->GetVal() == 0);

    for (unsigned int i = 2; i < 1000; i++)
    {
        id = map.Add(new TestObject());
        REQUIRE(id == i);
        t = map.Get(id);
        REQUIRE(t != nullptr);
        REQUIRE(*t != nullptr);
        (*t)->SetVal(id); // set the val to the id.
    }

    // The ids should be 2 to 999
    for (unsigned int i = 2; i < 1000; i++)
    {
        t = map.Get(i);
        REQUIRE(t != nullptr);
        REQUIRE(*t != nullptr);
        REQUIRE((*t)->GetVal() == (int)i);
    }
    // Test Getting a nonexistent obj
    t = map.Get(987654);
    REQUIRE(t == nullptr);

    // Test Removing an obj.
    TestObject **t5 = map.Remove(5);
    REQUIRE((*t5)->GetVal() == 5);
    // Test Getting obj 5 should now fail.
    t = map.Get(5);
    REQUIRE(t == nullptr);

    // Test find id by obj
    TestObject **t10 = map.Get(10);
    REQUIRE((*t10)->GetVal() == 10);
    id = map.GetIDFor(*t10);
    REQUIRE(id == 10);

    id = map.GetIDFor(*t5);
    REQUIRE(id == MYRIAD_INVALID_ID);
}
