This is a test example using ECS (currently ECS2) for objects.

I drew an architecture diagram of the engine, it is inaccurate, but it looked like this:

+------------------------------------------------+
|                 Client Code                    |
+================================================+
| logs| scene| project|game engine|console| stats|
+------------------------------------------------+
|  ECS2    |   S5    | EVENT SYSTEM    | THREADS |
+------------------------------------------------+
| Asset Manager       |  Media ? Interface       |
+---------------------+--------------------------|
|shdr|mdl|tex|fnt|snd | render|window|input|audio|
+================================================+
|                   RAYLIB                       | <-- this could be SDL2, or Vulkan+Input+Sound, whatever
+------------------------------------------------+

Of course this is not right, this is more of a vomit
of the things that are roughly in each layer, not
how they are layered or dependent.

It's worth figuring out now what calls client code actually makes.
1. What does game setup / loading look like?
2. What does a game loop look like?
3. What does that look like under threading?

1.
- LoadEngineConfig - Tears down any existing engine infra and loads engine params, data structure sizes etc.
- InitEngine - sets up engine datastructures, reinits window, scene list etc
- LoadScene(scene, additive) - load a scene, optionally INTO the current scene, i.e. additively.
- UnloadScene(scene)

Scene:
- will objey start, update, render, etc,
- so could have OnStart, OnUpdate, OnRender, also OnLoad/OnLoaded and OnUnload/OnUnloaded
- we need
  - asset: Load, Unload, Get
  - entity:Create, Destroy, Query? (see below)
  - component: Add, Remove, Get?
  - query: Add, Remove
  - system: Add, Remove, Get, OnEvent
  - event: Subscribe, Unsubscribe
