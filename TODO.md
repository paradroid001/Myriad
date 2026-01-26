## Myriad TODO

### 20231015

-[ ] Add Doxygen support -[ ] Add a Test directory and unit tests -[ ] Event System - can not yet unsubscribe

### 20231002

-[x] Currently there's no way to clean / build only the vendor libs

- Fixed 20231015: `make cleanlibs` -[x] Built linux binary for Sample seems to expect the engine shared library (libmyriad.so) to be in a particular place / relative directory. Building with gcc/g++ doesn't seem to do this, but if I build with zig c++, it has this outcome.
- Fixed 20231015: Rejigged build chain, setting LD_LIBRARY_PATH in the same command as runnning the binary should work, i.e: `LD_LIBRARY_PATH=Sample/build/Desktop/Release/x86_64-linux-gnu Sample/build/Desktop/Release/x86_64-linux-gnu/sample` -[x] The windows build chain is now (suddenly?) broken:
- It should just work with: `make PROJECT=Sample TARGET_OS=windows` but when building raylib, this is suddenly producing .obj files instead of .o files, meaning the linker can't find objects.
- Suspicion: zig /clang is doing some weird caching, perhaps this worked the first time but won't work now. I manually renamed the objs to .o and relinked and it seemed to work, but then we ran into a different issue when the shared lib tried to link against libraylib.a:
  Linking myriad.dll for x86_64-windows-gnu on linux
  zig c++ -target x86_64-windows-gnu -o build/Desktop/Release/x86_64-windows-gnu/bin/myriad.dll build/Desktop/Release/x86_64-windows-gnu/obj/core/Camera.o build/Desktop/Release/x86_64-windows-gnu/obj/core/Component.o build/Desktop/Release/x86_64-windows-gnu/obj/core/GameObject.o build/Desktop/Release/x86_64-windows-gnu/obj/core/Log.o build/Desktop/Release/x86_64-windows-gnu/obj/core/MyrApplication.o build/Desktop/Release/x86_64-windows-gnu/obj/core/Transform.o build/Desktop/Release/x86_64-windows-gnu/obj/core/Window.o build/Desktop/Release/x86_64-windows-gnu/obj/core/core.o -shared -l raylib -Lbuild/Desktop/Release/x86_64-windows-gnu/bin -Wl,--out-implib,build/Desktop/Release/x86_64-windows-gnu/bin/myriad.lib
  LLD Link... lld-link: error: undefined symbol: timeBeginPeriod

  > > > referenced by /myriad/Engine/vendor/raylib/src/rcore.c:859
  > > > libraylib.a(rcore.o):(InitWindow)

  lld-link: error: undefined symbol: timeEndPeriod

  > > > referenced by /myriad/Engine/vendor/raylib/src/rcore.c:973
  > > > libraylib.a(rcore.o):(CloseWindow)

- Fixed 20231015: After a lot of back and forth this now works, but annoyingly zig cc outputs .obj files for all the raylib compiles, so the Engine make breaks when trying to archive the .o files for libraylib.a. There is a line in the makefile to rename the .obj files to .o, so run the same make command again and the build will continue.

### 20260116

- I think all singletons should be removed. Except perhaps a gameengine or application one. Maybe. You still can't control the deletion of the static pointer. std::shared may fix this? Probably still not a good enough reason.
- The event system needs to support more usecases and possibly needs a major refactor.

  - It shouldnt't be a singleton (It's not, but it has some static vars...), It should expect multiple event systems are established.
  - Use cases:

    1. I don't really want to have to care about which event system I am emitting to when I create/emit an event. But at register time, it's probably fine to specify the event system. then you could have an event registered to multiple event systems (just call it multiple times?)
    1. A relatively small number of observers subscribe to an event.
    1. Events that get passed through every component instance (so all instances of a class?) which has subscribed to it. This is more like a broadcast. The specifics are to deliver it to every component on all gameobjects - the gameobjects could be grouped, perhaps.

    - Broadcast an event to everybody 'end of turn'
    - Broadcast an event to all enemies, and let their components deal with it one by one, altering the event until it drops out the bottom.

    1. Events that are sent to just the objects involved in an interaction - say, a collision.
    1. A mix - perhaps I want to target to 2 gameobjects (a collision) and any other subscribed function (say, a stats function or something)

    So:

    ```
    // Make an event system
    EventSystem es1 = new EventSystem();
    // Register an event - an observer (use case 1)
    es1.Register<CustomEvent1>(instance, method);
    // Create a custom event
    CustomEvent1 event1 = new CustomEvent1();
    event1.some_data = 42;
    //All observers will get called next EventSystem::ProcessEvents
    event1.Emit();

    // Make a broadcasting event system
    BroadcastEventSystem es2 = new BroadcastEventSystem();

    ```
### 20260126 Happy Aus Day!
Pre GGJ Checklist

1. No warnings in Myriad code, full warnings on in gcc. May require building some dependencies (spdlog?) with flags to supress.
2. Build working for Linux, Windows, Web. I don't care about Mac or Android right now.
3. Ability to build myriad statically, link your target game against static myriad.
4. Track allocations, stats, timings.
5. Tests for what can be unit tested.
6. Fix events, make eventsystem own and release them, consider multiple event systems (as long as events don't go between them?)
7. Multithreading: put the core of this back in the engine, even if only a single thread is used now. The biggest issue was web builds, would need some way of making the system think it is not multithreaded for web builds only.
8. Layers - both render order (back to front) as well as events (don't propagate down unless you have to). Physics layers would be different (more like groups).
9. Collisions - We don't have collisions yet, or entities dying / being removed. Watch out for dangling observers.
10. Scenes - a way of associating content and a state machine.
11. Serialisation - a way of specifying entities as data, not code/classes
12. Relocatable resources directory
13. Editor - only makes sense after scenes serialisation
14. Game Settings / Parameters as a file.
15. Switch fullscreen to windowed, window resizing, project settings shows allowable configurations.
