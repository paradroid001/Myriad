#ifndef TEST_OBJECTSTEST_H
#define TEST_OBJECTSTEST_H

#include "myriad.h"

#include <string>
#include <cassert>

class ObjectsTest
{
public:
  void Run(Myriad::Allocator &allocator)
  {
    Myriad::MyrObjectManager *p_object_manager = new Myriad::MyrObjectManager();
    auto myrobject = allocator.Alloc<Myriad::MyrObject>("MyrObject", p_object_manager);
    // destricts as a myrobject, because thats what comes through to allocator.
    auto gameobject = allocator.Alloc<Myriad::GameObject>("GameObject", p_object_manager);

    Myriad::MyrHandle<Myriad::GameObject> gameobj1 = allocator.Alloc<Myriad::GameObject>("1", p_object_manager);
    Myriad::MyrHandle<Myriad::GameObject> gameobj2 = allocator.Alloc<Myriad::GameObject>("2", p_object_manager);
    Myriad::MyrHandle<Myriad::GameObject> gameobj3 = allocator.Alloc<Myriad::GameObject>("3", p_object_manager);

    // TODO: this is totally going to break because these need their own
    // manager. But for the sake of compiling, I'll make the syntax work so that
    // these use the correct MyrHandle_T ids.
    // So just in case this gets run, I will put a failing assert in here.
    assert(1 == 0);
    /*
    gameobj1->AddChild(&*gameobj2);
    gameobj1->AddChild(&*gameobj2); // should be error
    gameobj2->AddChild(&*gameobj3);
    */
    gameobj1->AddChild(gameobj2.Handle());
    gameobj1->AddChild(gameobj2.Handle()); // should be error
    gameobj2->AddChild(gameobj3.Handle());
  }
};

#endif
