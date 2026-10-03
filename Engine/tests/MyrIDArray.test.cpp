#include "MyriadTypes.h" //do I need this for definition of random static global?
#include "myriad.h"      //MyrIDArray
#include <catch2/catch_test_macros.hpp>
#include <iostream>

TEST_CASE("MyrIDArray Add and Get", "[MyrIDArray]")
{
    Myriad::MyrIDArray<int, 8> array;
    auto id = array.Add(15);

    REQUIRE(id != MYRIAD_INVALID_ID);
    REQUIRE(array.Exists(id));
    REQUIRE(array.Get(id) != nullptr);
    REQUIRE(*array.Get(id) == 15);
    REQUIRE(array.Size() == 1);
}

TEST_CASE("MyrIDArray Reuse freed slot", "[MyrIDArray]")
{
    Myriad::MyrIDArray<int, 4> array;
    auto id1 = array.Add(1);
    auto id2 = array.Add(2);
    auto index_id1 = array.GetIndexForID(id1);
    REQUIRE(array.Exists(id1));
    REQUIRE(array.Exists(id2));

    auto removed = array.Remove(id1);
    REQUIRE(removed != nullptr);
    REQUIRE_FALSE(array.Exists(id1));
    REQUIRE(array.Size() == 1);

    auto id3 = array.Add(3);
    auto index_id3 = array.GetIndexForID(id3);
    // ids must continue to be unique
    REQUIRE(id3 != id1);
    REQUIRE(array.Exists(id3));
    REQUIRE(*array.Get(id3) == 3);
    REQUIRE(array.Size() == 2);
    // index should have been reused
    REQUIRE(index_id3 == index_id1);
}

TEST_CASE("MyrIDArray Iterate occupied entries", "[MyrIDArray]")
{
    Myriad::MyrIDArray<int, 4> array;
    auto id1 = array.Add(10);
    auto id2 = array.Add(20);
    auto id3 = array.Add(30);

    int sum = 0;
    int count = 0;
    for (auto it = array.begin(); it != array.end(); ++it)
    {
        sum += *it;
        count += 1;
    }

    REQUIRE(count == 3);
    REQUIRE(sum == 60);
    REQUIRE(array.Size() == 3);
}

TEST_CASE("MyrIDArray Remove invalid id returns nullptr", "[MyrIDArray]")
{
    Myriad::MyrIDArray<int, 4> array;
    REQUIRE(array.Remove(123u) == nullptr);
}

TEST_CASE("Add to capacity then remove then add", "[MyrIDArray]")
{
    Myriad::MyrIDArray<int, 20> array;
    Myriad::MYR_ID_t last_id;
    for (int i = 0; i < 20; i++)
    {
        last_id = array.Add(i);
    }
    REQUIRE(array.Size() == 20);
    int sum = 0;
    for (auto it = array.begin(); it != array.end(); ++it)
    {
        sum += *it;
    }
    REQUIRE(sum == 190); //== 19+18+17...+2+1+0 == 19*10

    for (int i = 0; i < 10; i++) // these are actual ids
    {
        array.Remove(i);
    }
    // Have removed the 0 to 10, == 45
    sum = 0;
    for (auto it = array.begin(); it != array.end(); ++it)
    {
        sum += *it;
    }
    REQUIRE(sum == 190 - 45);
    REQUIRE(array.Size() == 10);
    for (int i = 0; i < 10; i++)
    {
        REQUIRE_FALSE(array.Exists(i));
    }
    for (int i = last_id + 1; i < last_id + 11; i++)
    {
        array.Add(i);
    }
    REQUIRE(array.Size() == 20);
    // We would have added ids 20 through 29
    for (int i = 20; i < 30; i++)
    {
        REQUIRE(array.Exists(i));
    }
    sum = 0;
    for (auto it = array.begin(); it != array.end(); ++it)
    {
        sum += *it;
    }
    // Sum of 20 through 29 == 29*5 = 245
    REQUIRE(sum == 190 - 45 + 245);
}

TEST_CASE("Benchmarking")
{
    struct TestObject
    {
        int x;
        int y;
        bool alive;
    };

    Myriad::MyrTimer t;
    Myriad::MyrRandom rnd;
    const int max_objects = 20000;
    int random_numbers[max_objects];

    // Random numbers between 0 and maxobjects
    for (int i = 0; i < max_objects; i++)
    {
        random_numbers[i] = (int)rnd.Float(0, max_objects - 1);
    }
    Myriad::MyrIDArray<TestObject, max_objects> array;

    t.Start();
    // add objects
    for (int i = 0; i < max_objects; i++)
    {
        array.Add(
            {random_numbers[i], random_numbers[max_objects - 1 - i], true});
    }
    t.Stop();
    std::cout << "Adding " << max_objects << " objects, time: " << t.Elapsed()
              << std::endl;

    t.Start();

    // Iterate over the objects.
    float total_x = 0;
    float total_y = 0;
    for (auto s : array)
    {
        total_x += s.x;
        total_y += s.y;
    }
    t.Stop();
    std::cout << "Iterated " << array.Size()
              << " objects, total_x = " << total_x << " total_y = " << total_y
              << " time: " << t.Elapsed() << std::endl;

    // Remove half
    for (int i = 0; i < max_objects / 2; i++)
    {
        // remove a random array element.
        array.Remove(random_numbers[i]);
    }
    t.Stop();
    std::cout << "Removing " << max_objects / 2
              << " objects, time: " << t.Elapsed() << std::endl;

    t.Start();

    // Iterate over the objects.
    total_x = 0;
    total_y = 0;
    for (auto s : array)
    {
        total_x += s.x;
        total_y += s.y;
    }
    t.Stop();
    std::cout << "Iterated " << array.Size()
              << " objects, total_x = " << total_x << " total_y = " << total_y
              << " time: " << t.Elapsed() << std::endl;

    REQUIRE(true);
}
