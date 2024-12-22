# Myriad

Myriad Engine

## Building

main tasks in vscode will cmake and build for linux.
have to use command line (from base Myriad dir) to build for windows:
cmake -D CMAKE_TOOLCHAIN_FILE=TC-windows.cmake .
make
bin will end up in Test/

- See if I can make zig a bit more ubiquitous across platforms.
  - See https://www.youtube.com/watch?v=kuZIzL0K4o4
  - Maybe: https://zig.news/kristoff/cross-compile-a-c-c-project-with-zig-3599
  - Also, raylib with zig: https://gist.github.com/mr-ema/d78ec7fefb8ea1ed8b2907eb2f0dc9da
  - Raylib Template + box2d and ldTk: https://github.com/tupini07/raylib-cpp-cmake-template

## Stuff

- A Service is a persistent group of functionality which runs for the entire engine uptime, and is something we want to be globally accessible. Once it's allocated, it doesn't change: it will be a static pointer.

- A Provider is an abstraction over a concrete implementation such as a renderer, audio, physics, etc. This allows different implementations and libraries to be plugged in.

  - raylib can then be a rendering provider (and windowing - these will probably always be pretty linked.)
  - could write my own providers
  - would be cool to pull in Sokol: https://github.com/floooh/sokol
  - box2c as a physics provider? https://github.com/erincatto/box2c
  - spine is an animation framework: https://en.esotericsoftware.com/spine-in-depth

- A Handle wraps a pointer, handles can be passed around and copied, and the internal pointer is reference counted.

  - Sokol author likes Handles: https://floooh.github.io/2018/06/17/handles-vs-pointers.html
  - Based on https://bytes.com/topic/c/651599-handle-classes-smart-pointer
  - Currently this is not threadsafe
  - Is implicitly tied to the Allocator which hands out handles.

- The AllocatorService allocates and distributes Handles, and they live on in this scope. The AllocatorService is therefore responsible for cleaning up Handles and their associated pointers at Engine Shutdown.

  - Currently not very advanced: something better could be https://www.youtube.com/watch?v=BQC5WH_t-Uc
    - In short, use vectors and ids on the handle to invalidate them.
    - See this discussion about performance of vectors and arrays: https://stackoverflow.com/questions/381621/using-arrays-or-stdvectors-in-c-whats-the-performance-gap
  - Possibly look at Memory Pool implementations: https://www.geeksforgeeks.org/what-is-a-memory-pool/

- The Logging Service directs messages to multiple endpoints.
  - See: https://www.youtube.com/watch?v=DQCkMnMNFBI

### The job system:

- Currently a thread pool with a shared queue
  - Based on https://www.geeksforgeeks.org/thread-pool-in-cpp/
- Since so much code is not thread safe yet, this is restricted to one thread (!)
- Sounds useless but it's just to get the code to use a job submission style.
- Later on, the job system could be something better (It just needs to conform to the IJobSystem interface), like:
  - https://www.rismosch.com/article?id=building-a-job-system (in rust but references a c++ guy?)
  - https://wickedengine.net/2018/11/simple-job-system-using-standard-c/
  - Look at thread safe queues: https://www.geeksforgeeks.org/implement-thread-safe-queue-in-c/
- Really good talk on jobs and dependencies is: https://www.youtube.com/watch?v=1sAR3WHzJEM
- The Destiny talk on multithreading their engine is good (references the above)
- This talk on coroutines in a job system is good: https://www.youtube.com/watch?v=1ZMasi_9g_A
- This seems like a nice simple task system: https://github.com/p-ranav/task_system
- Taskflow also looks good: https://taskflow.github.io/taskflow/
- Or would jobxx be better?: https://github.com/seanmiddleditch/jobxx
- Interesting concurrency library: https://github.com/David-Haim/concurrencpp

### Entity Component System

- This guy made one: https://www.youtube.com/watch?v=dEdFM0uQpA0

### Embedding another language:

- for LUA see:
  - https://github.com/mormert/jle
  - JavidX has a series.

## Rules / Standards

### ("modern c++")

- use auto range for loops
- for new use make_unique by default, make_shared if it will be shared.
  - then for delete, you don't need to do anything.
  - non-owning pointers (something you don't need to change lifetime of) is still fine to pass \*/&
  - in fact passing non-owning non-raw pointers (i.e. refcounted pointers which will inc/dec every call) will be way slower.
- prefer auto. auto x = 42;
  - want to say the type? auto x = int{42};

### (mine):

- don't use c style casts if possible, they suck: https://anteru.net/blog/2007/c-background-static-reinterpret-and-c-style-casts/
  - converting along an inheritance chain? Use static_cast<T>

## Performance

- See Sokol Author: https://floooh.github.io/2018/05/01/cpp-to-c-size-reduction.html
- Size profiling for binaries: https://github.com/google/bloaty
- Use Valgrind
- For posterity, JobbedTest (500 objects), AllocatorQD, running for about 2 seconds,
  allocator destroyed 508 objects. Ran in WSL, make sure you install mesa-common-dev so you have glx, and export DISPLAY=:0
  Output of valgrind --tool=cachegrind --cache-sim=yes build/Test/MyriadTest

  ```
  ==10038==
  ==10038== I   refs:      2,791,171,291
  ==10038== I1  misses:       18,859,714
  ==10038== LLi misses:          414,819
  ==10038== I1  miss rate:          0.68%
  ==10038== LLi miss rate:          0.01%
  ==10038==
  ==10038== D   refs:      1,572,680,682  (1,058,955,945 rd   + 513,724,737 wr)
  ==10038== D1  misses:       26,223,736  (   15,522,069 rd   +  10,701,667 wr)
  ==10038== LLd misses:        3,710,625  (    1,575,701 rd   +   2,134,924 wr)
  ==10038== D1  miss rate:           1.7% (          1.5%     +         2.1%  )
  ==10038== LLd miss rate:           0.2% (          0.1%     +         0.4%  )
  ==10038==
  ==10038== LL refs:          45,083,450  (   34,381,783 rd   +  10,701,667 wr)
  ==10038== LL misses:         4,125,444  (    1,990,520 rd   +   2,134,924 wr)
  ==10038== LL miss rate:            0.1% (          0.1%     +         0.4%  )
  ```

  Also from the same binary, just running valgrind:

  ```
  ==10056==
  ==10056== HEAP SUMMARY:
  ==10056==     in use at exit: 30,595,813 bytes in 16,526 blocks
  ==10056==   total heap usage: 110,362 allocs, 93,836 frees, 58,540,165 bytes allocated
  ==10056==
  ==10056== LEAK SUMMARY:
  ==10056==    definitely lost: 78,136 bytes in 507 blocks
  ==10056==    indirectly lost: 20,000 bytes in 500 blocks
  ==10056==      possibly lost: 4,501,941 bytes in 2,555 blocks
  ==10056==    still reachable: 25,995,736 bytes in 12,964 blocks
  ==10056==         suppressed: 0 bytes in 0 blocks
  ==10056== Rerun with --leak-check=full to see details of leaked memory
  ==10056==
  ==10056== Use --track-origins=yes to see where uninitialised values come from
  ==10056== For lists of detected and suppressed errors, rerun with: -s
  ==10056== ERROR SUMMARY: 10000000 errors from 503 contexts (suppressed: 0 from 0)
  ```

## TODO:

- Cleanroom branch
  resource loader
- font
- textures, sounds
  job system
- with stats/timing
- shared library engine and app, with stub loader.
  - allow hot reloading.

Issues:

1. OpenGL context has to be inited on the same thread as rendering.
2. Have to guarantee the init job runs before any others.
3. For raylib at least, you have to init textures after GL is inited, i.e. after the render init job.

# Projects and Links:

Interesting GL/Multiplayer: https://github.com/calint/glos?tab=readme-ov-file
Valgrind Driven dev: https://atilaoncode.blog/2015/06/22/valgrind-driven-development/
Valgrind's Tools: https://valgrind.org/info/tools.html
How To Profile C++ with Callgrind/KCacheGrind: https://baptiste-wicht.com/posts/2011/09/profile-c-application-with-callgrind-kcachegrind.html
This is also good: https://stackoverflow.com/questions/32542164/using-valgrind-to-measure-cache-misses
Set DISPLAY in WSL: https://askubuntu.com/questions/1299323/how-to-set-up-display-variable-for-wsl2-of-ubuntu-20
Handles are better pointers: https://floooh.github.io/2018/06/17/handles-vs-pointers.html
CMake Dependencies: https://floooh.github.io/2016/01/12/cmake-dependency-juggling.html
Building an ECS series: https://ajmmertens.medium.com/building-an-ecs-2-archetypes-and-vectorization-fe21690805f9
This guy didn't like Raylib: https://danielchasehooper.com/posts/shapeup/
Interesting fantasy platform thing: https://bztsrc.gitlab.io/meg4/
ET Engine - Planet Vis: http://leah-lindner.com/blog/et_engine/
ET Engine - Github: https://github.com/Illation/ETEngine

Mike Acton - Data Oriented Design and C++: https://www.youtube.com/watch?v=rX0ItVEVjHc
Game Engine using C++11: https://www.youtube.com/watch?v=8AjRD6mU96s

This is an interesting post/engine:

- https://phoboslab.org/log/2024/08/high_impact
- https://github.com/phoboslab/high_impact

# Notes

## Gameobject-Component model:

### GameObject

    Children: GameObject[] (authoring my own would just be data, because they only vary by component?)
    Components: Component[] (these could be anything, you would want to define them in your app)

So what if we just define interfaces for now, and then have a few implementations:

- a quick and dirty implementation (do whatever)
- a performance conscious implementation where I might want to try using a few perf conscious ways of doing things.
  - making structs small, looking at alignment, arrays.
- an ecs version using flecs or similar.

The aim, particularly of doing the quick and dirty version, is to actually start making a game, and worrying about performance later.

### Alloc Requirements:

- From any user/engine code, I want you to construct me a Thing and hand me back a token (handle) that I can use to get that Thing in future.
- I can ask for that Thing to be destroyed, if I have the token.
- I can swap out the backend implementation that manages Things.
- My token knows if it has become invalid.

Handle h1 = allocator.Alloc<GameObject>(pos,rot,scale);
Handle h2 = allocator.Alloc<TextObject>(x, y, "Hello World");

allocator.Alloc<GameObjec>(pos, rot, scale);

## So, thoughts 20240811:

- No singletons - if something needs access to a thing, it gets passed in.
  - therefore there can be permissions around this: you can pass things in const etc.
- Get multithreading working first, because that sorts out all the permission stuff (passing/injection of things that are needed).

## TODO 20241005 (at GCAP)

- I want there to be a global app state or object that holds

  - allocators, object managers, threadpool
  - init it with a prefs object, that affects how the services are initialised.
  - the subsystems (allocators, managers, etc) should not be singletons. I want to control their lifetime, and the capacity of the allocators (for example)
  - I guess the game should be able to be single or multi threaded.

  ## TODO 20241122

  SO:
  We're just not going to be able to guarantee that web will support threads.
  We can still come up with some smart 'submit things to the runloop' with deppendencies,
  which will work in threaded and nonthreaded environments, there should probably
  just be an option to do it yourself, custom, however you want.
  Seems like the easiest path would be to have base methods in MyriadApplication,
  being Init and RunLoop or something similar, and being able to override
  them if you want.

The engine should have a few overriding design principles:

1. By default it can initialise everything, provide global systems (allocator, resource manager, etc) and provide a threaded or nonthreaded job system and runloop to which you can submit jobs if you like. BUT. You should always be able to override - I don't want the standard init, I don't want the standard runloop, etc.

Other things that need to be done:

1. Separate out all the headers into an include directory, so client apps can include this bundle.
2. Logging probably needs to be sorted out, once and for all.
3. Config files and other serialisation/deserialisation is probably a good idea, nice and early.
4. Can probably remove the runtime type id system, although components haven't been heavily tested..
5. Events get created in application code with new, and never deleted. May need event manager to pool and reuse them.
6. The situation with handles is a bit unweildy if you just want a big vector full of all your handles, of all different types. Makes pointers seem far better. Is there a better way?
