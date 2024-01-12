#include "rendering/Renderer.h"
#include "core/Transform.h"
#include "core/Types2D.h"
#include "core/core.h"
#include "io/Log.h"
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
        ::Vector2 raylib_p1 = {p1.x, p1.y};
        ::Vector2 raylib_p2 = {p2.x, p2.y};
        ::Vector2 raylib_p3 = {p3.x, p3.y};

        DrawTriangle(raylib_p1, raylib_p2, raylib_p3,
                     {colour.r, colour.g, colour.b, colour.a});
    }
    // Static
    void Renderer::MyrDrawText(const char *text, float x, float y,
                               float lineheight, Myriad::MyrColour colour)
    {
        DrawText(text, x, y, lineheight,
                 {colour.r, colour.g, colour.b, colour.a});
    }
} // namespace Myriad