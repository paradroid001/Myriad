#include "core/Renderer.h"
#include "core/Transform.h"

// Only include raylib if we're building internally.
#ifdef MYRIAD_INTERNAL
    #include "raylib.h"
#endif

void Myriad::Renderer::Draw(Myriad::TransformData t)
{
    DrawCircle(t.position.x, t.position.y, 5, {100, 50, 0, 255});
}
