#include "myriad.h"

#include "JobbedTest.h"
#include "ScheduledJobbedTest.h"
#include "TestThreadPool.h"
#include "NonJobbedGameTest.h"
#include "ObjectsTest.h"

class MyriadTest : public Myriad::MyrApplication
{
public:
  Myriad::AllocatorService *allocator_ptr;
  Myriad::MyrAppPreferences prefs;

  MyriadTest()
  {
    MYR_INFO("Hello this is an info message");
    std::cout << "Hello world from myr test" << std::endl;
    allocator_ptr = new Myriad::AllocatorService();
    MYR_TRACE("Made an allocator");
  }

  ~MyriadTest()
  {
    std::cout << "myr test destructor" << std::endl;
    allocator_ptr->Shutdown();
    delete allocator_ptr;
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
    Myriad::MyrHandle<NonJobbedGameTest> njgt = allocator_ptr->Alloc<NonJobbedGameTest>();
    njgt->Run(*allocator_ptr, prefs);
  }

  void BasicThreadPoolTest()
  {
    Myriad::MyrHandle<TestThreadPool> ttp = allocator_ptr->Alloc<TestThreadPool>();
    ttp->Run(*allocator_ptr);
  }

  void Nicer()
  {
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
    auto objtest = allocator_ptr->Alloc<ObjectsTest>();
    objtest->Run(*allocator_ptr);
  }

  void Run()
  {
    prefs.screen_dimensions = {800, 600};
    prefs.target_fps = 60;
    // The basic, non jobbed game test
    // BasicNonJobbedGameTest();

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
};

Myriad::MyrApplication *Myriad::CreateApplication()
{
  return new MyriadTest();
}
