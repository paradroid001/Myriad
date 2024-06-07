# Myriad

Myriad Engine

If you want debug tasks etc, copy Projects/vscode/.vscode into your .vscode

## Quick Top Level Build

Easiest is to build and bring up the docker container with the build environment:

- Build: cd docker && docker build -t myriad_build .
- Run (from root): source run_docker.sh

Default compiler is Zig, but this seems to produce binaries that want the SO in a specific location. Using g++ fixes this on Linux:

`make PROJECT=Engine TARGET_OS=linux CXX=g++ all`
`make PROJECT=Sample TARGET_OS=linux CXX=g++ all`

Building cross compile for Windows can use zig just fine:

`make PROJECT=Engine TARGET_OS=windows all`
`make PROJECT=Sample TARGET_OS=windows all`

You can also pass:
TARGET_ARCH (default=x86_64)
TARGET_OS (default=linux, options=windows,linux)
RELEASE_PLATFORM (default=Desktop, options=Desktop,Web,Android)
BUILD_MODE (default=Release, options=Debug,Release)

zig is now used as the default compiler (`zig c++ -target $(TARGET_ARCH)-$(TARGET-OS)`)
If you really want to use g++, you can pass CXX=g++
This is useful for generating compile_commands.json via bear:
cd Sample
`bear -- make CXX=g++ all`

The below instructions are now out of date.

### Linux Build

Build myriad first, then build the sample.
Run from the correct directory, with the LD_LIBRARY_PATH set.

```
cd Engine
TARGET_PLATFORM=PLATFORM_DESKTOP make clean
TARGET_PLATFORM=PLATFORM_DESKTOP make all
cd ..
cd Sample
make all
cd build
LD_LIBRARY_PATH=. sample
```

### Windows Build

#### New Instructions

If you are not using raylib directly in your 'client' app, you can do:

```
PROJECT=Engine TARGET_OS=windows cleanlibs
PROJECT=Engine TARGET_OS=windows clean
PROJECT=Engine TARGET_OS=windows make
```

you may need to re-run this command as .obj files need to be renamed to .o

then

```
PROJECT=XXX TARGET_OS=windows make clean
PROJECT=XXX TARGET_OS=windows make
```

If you are using raylib functions directly in your client app, you need to build in a strange way:

```
PROJECT=Engine TARGET_OS=windows make cleanlibs
PROJECT=Engine TARGET_OS=windows make clean
PROJECT=Engine TARGET_OS=windows RAYLIB_LIBTYPE=SHARED make
```

This link will fail because of some dynamic lib paths. Continue without RAYLIB_LIBTYPE=SHARED:

```
PROJECT=Engine TARGET_OS=windows make
```

Then build your client as above. Should work fine.

#### Old Instructions

This works best with w64devkit, from https://github.com/skeeto/w64devkit
Just clone the repo, build the docker image, and then unzip the devkit somewhere convenient:

```
docker build -t w64devkit .
docker run --rm w64devkit >w64devkit.zip
mv w64devkit.zip c:\compilers
cd c:\compilers
unzip w64devkit.zip
```

In the shell you will run `make` from, set your path:
`set PATH=c:\compilers\w64devkit\bin;%PATH%`

now you can make Myriad:

```
cd Engine
TARGET_PLATFORM=PLATFORM_DESKTOP make clean
TARGET_PLATFORM=PLATFORM_DESKTOP make all
cd ..
```

And now the sample, note we must rename to dll:

```
cd Sample
make all
cd build
mv libmyriad.so libmyriad.dll
sample.exe
```

# Coding Style

- Google Style Guide is here: https://students.cs.byu.edu/~cs235ta/references/Cpp%20Style/Google%20Cpp%20Style%20Guide.pdf
  - need to come to some agreement. Basically I just want to know if they prefix pointers with p or p\_ or something, and if private members have a leading or trailing underscore or some other marker.
- Classes should be named PascalCase, i.e. RendererGroup, InputManager
- Variable names are camelCase, i.e. distanceToTarget.
- Member variables don't require m\_, they're just normal names, including pointers.
- Braces { } on next line, even for trivial functions { return x; }

# Dev priorities

- Read config files, possibly serialise config files also
  - Needed to configure logging and switch on/off other runtime behaviour

# Research

## Threading: Ron Fosner (2010) - https://www.youtube.com/watch?v=1sAR3WHzJEM

- try to keep GPU and CPU busy
- Use Tasks, not threads (threads are just tools for your tasks)
- Think about your architecture as a task based system: tasks don't know how many threads there are.
- "Jobs" should be broken up into "Tasks" if possible - you're trying to saturate the hardware threads

A TASK is independent code and should be stateless, i.e. it's a function call, the only context it has is what is passed in.

- a black box that you can fire up, does it's thing, and is retired.
- it's independent in that it doesn't "call out" to code that is in other tasks, but it might be dependent on those tasks having already run (this frame)
- num tasks <= hardware threads.
- if you're churning a lot of data you want to break up into tasks if you can because you want to use "fault sharing". Not too long running (>30ms) either or OS swaps them out. Not too small that they 'pollute the l2 cache'.
- Tasks need three things:
  1.  block if dependencies are pending (the thread pool swaps this thread out in this case)
  2.  do work pCurrentTask->doWork(pCurrentTask->context());
  3.  signal to dependents that you are done. (just decrements a counter in those tasks)
- Mutex vs Critical Section vs Slim Reader/Writer Lock (? is the latter MS or intel specific?)

class ITask
{
protected:
vector<ITask*> m_dependentsList; //those waiting for me
ThreadSafe_Int32 m_dependenciesRemaning; //dependents I am waiting for (threadsafe counter)
SRWLOCK m_SRWLock; //the blocking event if we have dependencies we are waiting on
CONDITION_VARIABLE m_Dependencies;
void SignalDependentsImDone();
ITask();
virtual ~ITask();
public:
void blockIfDependenciesArePending();
void isDependentUpon(ITask* pTask);
unsigned int queryNumberDependentsRemaining() //dependents are left, then I run (someone is waiting)
void clearOneDependency();
virtual void* doWork(void*) = 0;
virtual void\* context() = 0;
}

Goal: keep the cores busy. Create an architecture which allows you to distribute and schedule tasks efficiently across HW threads. - consider "exact" vs "close enough" - does it need to update every frame? (slow moving physics? particles?) - can scheduler flex some tasks, ramping down if the game is lagging?

Sorting is a good example with tasks (merge sort). Or recursive backtracking with maze solving.

Approach:

- Break up things into stages: UI, Physics, Rendering, etc.
- Break up stages into tasks, try to minimise dependencies between stages: know the difference between must-be-sequential and robust. (i.e. if somehow you started too late, rather than having locked some state to allow you to start late, you have some kind of test which tells you you were too late, and then just back out and try again next time). Obviously only works for certain 'doesn't have to be perfect' jobs.
- Should work on any number of threads, automatically scales as #cores increases, thread pools give you platform independence

Tips:

- Start off small, build up
- Converting to tasks: look at what can be done in parallel, reduce interdependency if possible, be thread safe from the start
- measure, measure, measure: instrument your code and use tools that allow you to see what is going on.
- you WANT to be in the situation where you're dealing with cpu contention, l2 cache optimisation etc, because that means you solved the hard part and now you're optimising.

## Multithreading the Entire Destiny Engine (https://www.youtube.com/watch?v=v2Q_zHG3vqg)

- Most systems repeat the same work each frame, core set of jobs are often linear, have similar data access patterns.
- Formalised the above into first class citizen as a "Fiber", defined as a set of jobs that always run serially and sequentially with an associated block of memory.
  [Network State Memory] (accessible by all network jobs inside this fiber)
  Network Update -> Network Send -> Network Recieve
- One pass through the job chain is an iteration
- They are a convenience, easier to understand than a mess of jobs, know what to expect re: data access patterns, easy to describe cross system dependencies: documented in one system and not spread across the code (it just packages everything up)
- Time control thread (only real one?) kicks off fiber iterations every 33.36ms. Overhead of providing accurate timed execution within job system is too high. Responsible for serializing all fibers when needed.
- Serializing: You need to drain all the fibers, wait for async jobs to complete, switch to single-threaded-like-behaviour (serialize state out), and then fire back up. Time control thread executes this operation.
- Optimal HW utilisation:
  - try to keep platform independent, principles (at least) apply to all platforms.
  - one job thread per core usually.
  - simulation fiber is latency critical - high priority/affinity job thread to run these. (that is, get sim work done as quick as possible, without interruption)
- Async job threads:
  sometimes there are tasks you don't have control over, OS controls them, HTTP request, or some external system.
  may go to sleep or take locks
  try to keep data contained so you can serialise them (or other fibers) and you don't need to wait (no data dependency)

Prove The code is thread safe

- How do we robustly detect a whole class of thread/contention/corruption bugs without them hitting the wild?
- A system to validate resources are used with thread safe patterns - "Execution Environment"
  - Identify unsafe patterns during development, highlight potential issues without hitting them, detect invalid access patterns leading to:
    - Multithreaded non-determinism
    - Data race conditions
    - Rare non-reproducible crashes
  - It's verification only, this isn't a shippable thing
  - A 'resource' is something that can be accessed (that needs to be protected in MT env) - example Havok World, Lua VM, player profile, AI data
  - Policy 'markup' is added into the code to say what is allowed to acces what. Do a lookup in the policy on each resource usage. Assert when we don't have access. Requires good code coverage.
  - So if you were raycasting, and updating the havok world at the same time, that's going to assert. (policies overlap, but both can't be active at the same time). These are to detect race conditions, you may not have actually hit it.
  - Checked when a job is addded to the job graph against jobs that could potentially run concurrently under any scheduling permutaion (so it's speculative)
- Bungie consider this Execution Environment "essential" - couldn't have shipped on 4 platforms with no known MT bugs without it.

## Also see:

Destiny's Multithreaded Rendering Architecture (Natalya Tatarchuk) - https://www.youtube.com/watch?v=0nTDFLMLX9k&list=PLkyNcUdU5P2kWUIiEuE9OeC6fOlBMGFKT&index=3
Lessons from the Core Engine Architecture of Destiny (Chris Butcher) - https://www.youtube.com/watch?v=UU92a5pKV6k&t=152s
Destiny's Networked Mission Architecture (Justin Truman) (https://www.youtube.com/watch?v=Iryq1WA3bzw&pp=ygUuRGVzdGlueSdzIE11bHRpdGhyZWFkZWQgUmVuZGVyaW5nIEFyY2hpdGVjdHVyZQ%3D%3D)
Parallelizing the Naughty Dog Engine Using Fibers (https://www.youtube.com/watch?v=HIVBhKj7gQU&pp=ygUSbmF1Z2h0eSBkb2cgZmliZXJz)

Handles v Pointers: https://floooh.github.io/2018/06/17/handles-vs-pointers.html
This library (similar to RAYLIB) looks incredible: https://github.com/floooh/sokol

Myriad:

App starts up:
create myr application
--- INIT ---
read config file
init event system - init inputs / kb etc?
init render backend - create window
init audio backend
init asset system
init job/task system
--- END INIT ---

    --- GAME ---

    ?

    --- QUIT GAME ---

    --- SHUTDOWN ---
    close window
    release job system
    unload assets, close open files
    release audio
    release render backend
    release event system
    close logging
    --- END SHUTDOWN ---
