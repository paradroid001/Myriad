# Myriad

Myriad Engine

## Building

main tasks in vscode will cmake and build for linux.
have to use command line (from base Myriad dir) to build for windows:
cmake -D CMAKE_TOOLCHAIN_FILE=TC-windows.cmake .
make
bin will end up in Test/

## Stuff

A Service is a persistent group of functionality which runs for the entire engine uptime, and is something we want to be globally accessible. Once it's allocated, it doesn't change: it will be a static pointer.

A Provider is an abstraction over a concrete implementation such as a renderer, audio, physics, etc. This allows different implementations and libraries to be plugged in.

A Handle wraps a pointer, handles can be passed around and copied, and the internal pointer is reference counted.

The AllocatorService allocates and distributes Handles, and they live on in this scope. The AllocatorService is therefore responsible for cleaning up Handles and their associated pointers at Engine Shutdown.

The Logging Service directs messages to multiple endpoints.

The job system:

- Currently a thread pool with a shared queue
- Since so much code is not thread safe yet, this is restricted to one thread (!)
- Sounds useless but it's just to get the code to use a job submission style.
- Later on, the job system could be something better (It just needs to conform to the IJobSystem interface), like:
  - https://www.rismosch.com/article?id=building-a-job-system (in rust but references a c++ guy?)
  - https://wickedengine.net/2018/11/simple-job-system-using-standard-c/
