#include "myriad.h"

#include "JobbedTest.h"
#include "ScheduledJobbedTest.h"
#include "TestThreadPool.h"
#include "NonJobbedGameTest.h"
#include "ObjectsTest.h"

#include "core/memory/AllocatorQD.h"

class MyriadTest : public Myriad::MyrApplication
{
public:
  Myriad::MyrAppPreferences prefs;

  MyriadTest()
  {
    MYR_INFO("Hello this is an info message");
    std::cout << "Hello world from myr test" << std::endl;
  }

  ~MyriadTest()
  {
    std::cout << "myr test destructor" << std::endl;
  }

  /*
  void Nice()
  {
    AllocatorService *allocator_service = CreateService<AllocatorService>();
    ResourceService *resource_service = CreateService<ResourceService>();
    Myr.core.SetAllocatorService(allocator_service);
    Myr.core.SetResourceService(resource_service);
    Myr.core.allocator_service->Start();
    Handle window = Myr.allocator->Alloc<WindowProvider>();
    Myr.rendering.SetWindowProvider(window);

    Myr.rendering.window.Init(800, 600, "My Window");

    Handle gameObject = Myr.core.allocator.Alloc();
  }
  */

  void BasicNonJobbedGameTest()
  {
    Myriad::Allocator *allocator_ptr = GetEngine()->GetAllocatorGeneral();
    Myriad::MyrHandle<NonJobbedGameTest> njgt = allocator_ptr->Alloc<NonJobbedGameTest>();
    njgt->Run(*allocator_ptr, prefs);
  }

  void BasicThreadPoolTest()
  {
    Myriad::Allocator *allocator_ptr = GetEngine()->GetAllocatorGeneral();
    Myriad::MyrHandle<TestThreadPool> ttp = allocator_ptr->Alloc<TestThreadPool>();
    ttp->Run(*allocator_ptr);
  }

  void Nicer()
  {
    Myriad::Allocator *allocator_ptr = GetEngine()->GetAllocatorGeneral();
    Myriad::MyrHandle<JobbedTestGame> jtg = allocator_ptr->Alloc<JobbedTestGame>();
    jtg->Init(&*allocator_ptr);
    jtg->Run(&*allocator_ptr, prefs);
  }

  void Nicest()
  {
    ScheduledJobbedTest sjt;
    sjt.Run();
  }

  void TestObjects()
  {
    Myriad::Allocator *allocator_ptr = GetEngine()->GetAllocatorGeneral();
    auto objtest = allocator_ptr->Alloc<ObjectsTest>();
    objtest->Run(*allocator_ptr);
  }

  void Run()
  {
    prefs.screen_dimensions = {800, 600};
    prefs.target_fps = 60;
    prefs.threaded = false;
    prefs.num_threads = 1; // I haven't figured out multithreaded yet.
    InitEngine(prefs);     // init the engine with the pre-populated prefs

    if (true || !prefs.threaded)
    {
      // The basic, non jobbed game test
      BasicNonJobbedGameTest();
    }
    else
    {
      // BasicThreadPoolTest();
      //    MYR_CORE_TRACE("Allocated jobs should have been deleted by now.");

      // An attempt to run render/update/get input on jobs in a simple
      // job system. This has been rigged to ONLY have one thread, therefore
      // the window init and gl rendering is all happening on the same thread.
      // IF you don't do this, you get a blank window, nothing will draw.
      Nicer();

      // Scheduled jobs with dependencies.
      // Nicest();

      // Test out object model
      // TestObjects();
    }
    ShutdownEngine();
  }
};

Myriad::MyrApplication *Myriad::CreateApplication()
{
  return new MyriadTest();
}

/*
OK.The task from here is :
1. Can I get res directory to be copied to the build dir ?
  See : https : // www.youtube.com/watch?v=sTuIRzrB2dA
2. Can I then edit the way stuff gets launched to use the right dir context when running the binary?
  Edit the kits, or settings.json, or something ?
3. Can I still maintain GDB when doing this ?
4. Can I have builds go to build dirs that are per platform / variant, e.g : build / x86_64_windows_gnu / Debug ?
  Can I then maintain GDB this way ? should be as simple as :
  "cmake.buildDirectory": "${workspaceFolder}/build/${buildKit}/${buildType}",
*/
