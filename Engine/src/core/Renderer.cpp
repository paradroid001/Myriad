#include "core/Renderer.h"
#include "core.h"
#include "core/Log.h"
#include "core/Transform.h"
#include "core/Types2D.h"
// Only include raylib if we're building internally.
#ifdef MYRIAD_INTERNAL
    #include "raylib.h"
#endif

namespace Myriad
{
    Renderer::~Renderer() { MYR_CORE_TRACE("Renderer destructor"); }
    void Renderer::Draw(Myriad::TransformData t)
    {
        DrawCircle(t.position.x, t.position.y, 5, {100, 50, 0, 255});
    }

    // Static
    void Renderer::MyrDrawCircle(float x, float y, float rad,
                                 Myriad::MyrColour colour)
    {
        DrawCircle(x, y, rad, {colour.r, colour.g, colour.b, colour.a});
    }

    // Static
    void Renderer::MyrDrawTriangle(Myriad::Vector2 p1, Myriad::Vector2 p2,
                                   Myriad::Vector2 p3, Myriad::MyrColour colour)
    {
        DrawTriangle(p1, p2, p3, {colour.r, colour.g, colour.b, colour.a});
    }
    // Static
    void Renderer::MyrDrawText(const char *text, float x, float y,
                               float lineheight, Myriad::MyrColour colour)
    {
        DrawText(text, x, y, lineheight,
                 {colour.r, colour.g, colour.b, colour.a});
    }
} // namespace Myriad