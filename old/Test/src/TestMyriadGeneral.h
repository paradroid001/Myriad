#ifndef TEST_TESTMYRIADGENERAL_H
#define TEST_TESTMYRIADGENERAL_H

#include "myriad.h"

class TestMyriadGeneral
{
public:
  void Run()
  {
    // So this would use the prefs to
    // init the renderer and window.
    // also the job queue, ready for jobs to
    // be submitted, but with all the render
    // jobs on the same thread.
    // ptrs etc would be in the
    //  INIT RENDERER
    //  INIT WINDOW
    //  INIT JOBS
    //  INIT GAME OBJECT MANAGER
    // Sets allocators for various classes

    // RunningEngineState = MyriadInit(prefs);

    // Create a new scene
    // Scene s = allocator->Alloc("My scene");
    // Add a game object
    // s.AddChild(GameObject::Alloc("My gameobject"));
  }
}

#endif
