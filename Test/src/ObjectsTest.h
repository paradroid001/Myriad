#ifndef TEST_OBJECTSTEST_H
#define TEST_OBJECTSTEST_H

#include "myriad.h"
#include <string>

class ObjectsTest
{
public:
  void Run(Myriad::Allocator &allocator)
  {

    auto myrobject = allocator.Alloc<Myriad::MyrObject>("MyrObject");
    // destricts as a myrobject, because thats what comes through to allocator.
    auto gameobject = allocator.Alloc<Myriad::GameObject>("GameObject");

    Myriad::MyrHandle<Myriad::GameObject> gameobj1 = allocator.Alloc<Myriad::GameObject>("1");
    Myriad::MyrHandle<Myriad::GameObject> gameobj2 = allocator.Alloc<Myriad::GameObject>("2");
    Myriad::MyrHandle<Myriad::GameObject> gameobj3 = allocator.Alloc<Myriad::GameObject>("3");

    gameobj1->AddChild(&*gameobj2);
    gameobj1->AddChild(&*gameobj2); // should be error
    gameobj2->AddChild(&*gameobj3);
  }
};

#endif
