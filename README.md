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

TODO:

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
