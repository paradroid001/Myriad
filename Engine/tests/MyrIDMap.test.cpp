#include "core/MyrContainers.h"
#include <catch2/catch_test_macros.hpp>
#include <string>
#include <unordered_map>

struct MyrIDMapTestObject
{
    int value;
    std::string name;

    bool operator==(const MyrIDMapTestObject &other) const
    {
        return value == other.value && name == other.name;
    }
};

TEST_CASE("MyrIDMap Add and Get", "[MyrIDMap]")
{
    Myriad::MyrIDMap<int> map;
    auto id1 = map.Add(10);

    REQUIRE(id1 != MYRIAD_INVALID_ID);
    REQUIRE(map.Exists(id1));
    REQUIRE(map.Get(id1) != nullptr);
    REQUIRE(*map.Get(id1) == 10);
    REQUIRE(map.GetIDFor(10) == id1);
}

TEST_CASE("MyrIDMap Add duplicate returns existing id", "[MyrIDMap]")
{
    Myriad::MyrIDMap<int> map;
    auto id1 = map.Add(7);
    auto id2 = map.Add(7);

    REQUIRE(id1 == id2);
    REQUIRE(id2 != MYRIAD_INVALID_ID);
    REQUIRE(map.Exists(id2));
    REQUIRE(map.GetIDFor(7) == id1);
}

TEST_CASE("MyrIDMap Add and Get class object", "[MyrIDMap]")
{
    Myriad::MyrIDMap<MyrIDMapTestObject> map;
    MyrIDMapTestObject object{42, "test"};

    auto id = map.Add(object);
    REQUIRE(id != MYRIAD_INVALID_ID);
    REQUIRE(map.Exists(id));

    auto *stored = map.Get(id);
    REQUIRE(stored != nullptr);
    REQUIRE(stored->value == 42);
    REQUIRE(stored->name == "test");
    REQUIRE(map.GetIDFor(object) == id);
}

TEST_CASE("MyrIDMap Add duplicate class object returns existing id",
          "[MyrIDMap]")
{
    Myriad::MyrIDMap<MyrIDMapTestObject> map;
    MyrIDMapTestObject object{5, "dup"};
    MyrIDMapTestObject duplicate{5, "dup"};

    auto id1 = map.Add(object);
    auto id2 = map.Add(duplicate);

    REQUIRE(id1 == id2);
    REQUIRE(id2 != MYRIAD_INVALID_ID);
    REQUIRE(map.Exists(id2));
    REQUIRE(map.GetIDFor(duplicate) == id1);
}

TEST_CASE("MyrIDMap Remove invalid id returns nullptr", "[MyrIDMap]")
{
    Myriad::MyrIDMap<int> map;
    REQUIRE(map.Remove(123u) == nullptr);
}

TEST_CASE("MyrIDMap Remove existing entry", "[MyrIDMap]")
{
    Myriad::MyrIDMap<int> map;
    auto id = map.Add(42);

    REQUIRE(map.Exists(id));
    auto removed = map.Remove(id);
    REQUIRE(removed != nullptr);

    REQUIRE_FALSE(map.Exists(id));
    REQUIRE(map.Get(id) == nullptr);
    REQUIRE(map.GetIDFor(42) == MYRIAD_INVALID_ID);
}

TEST_CASE("MyrIDMap iterates entries", "[MyrIDMap]")
{
    Myriad::MyrIDMap<int> map;
    auto id1 = map.Add(1);
    auto id2 = map.Add(2);
    auto id3 = map.Add(3);

    std::unordered_map<Myriad::MYR_ID_t, int> values;
    for (auto it = map.begin(); it != map.end(); ++it)
    {
        values[it->first] = it->second;
    }

    REQUIRE(values.size() == 3);
    REQUIRE(values[id1] == 1);
    REQUIRE(values[id2] == 2);
    REQUIRE(values[id3] == 3);
}
