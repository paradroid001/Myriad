## Myriad TODO ##

### 20231002 ###
- Currently there's no way to clean / build only the vendor libs
- Built linux binary for Sample seems to expect the engine shared library (libmyriad.so) to be in a particular place / relative directory. Building with gcc/g++ doesn't seem to do this, but if I build with zig c++, it has this outcome.
- The windows build chain is now (suddenly?) broken:
  - It should just work with: `make PROJECT=Sample TARGET_OS=windows` but when building raylib, this is suddenly producing .obj files instead of .o files, meaning the linker can't find objects.
  - Suspicion: zig /clang is doing some weird caching, perhaps this worked the first time but won't work now. I manually renamed the objs to .o and relinked and it seemed to work, but then we ran into a different issue when the shared lib tried to link against libraylib.a:

Linking myriad.dll for x86_64-windows-gnu on linux
zig c++ -target x86_64-windows-gnu -o build/Desktop/Release/x86_64-windows-gnu/bin/myriad.dll  build/Desktop/Release/x86_64-windows-gnu/obj/core/Camera.o  build/Desktop/Release/x86_64-windows-gnu/obj/core/Component.o  build/Desktop/Release/x86_64-windows-gnu/obj/core/GameObject.o  build/Desktop/Release/x86_64-windows-gnu/obj/core/Log.o  build/Desktop/Release/x86_64-windows-gnu/obj/core/MyrApplication.o  build/Desktop/Release/x86_64-windows-gnu/obj/core/Transform.o  build/Desktop/Release/x86_64-windows-gnu/obj/core/Window.o  build/Desktop/Release/x86_64-windows-gnu/obj/core/core.o -shared -l raylib -Lbuild/Desktop/Release/x86_64-windows-gnu/bin -Wl,--out-implib,build/Desktop/Release/x86_64-windows-gnu/bin/myriad.lib 
LLD Link... lld-link: error: undefined symbol: timeBeginPeriod
>>> referenced by /myriad/Engine/vendor/raylib/src/rcore.c:859
>>>               libraylib.a(rcore.o):(InitWindow)

lld-link: error: undefined symbol: timeEndPeriod
>>> referenced by /myriad/Engine/vendor/raylib/src/rcore.c:973
>>>               libraylib.a(rcore.o):(CloseWindow)
