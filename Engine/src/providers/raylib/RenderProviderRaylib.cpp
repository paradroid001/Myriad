#include "providers/raylib/RenderProviderRaylib.h"

namespace Myriad
{
    bool RenderProviderRaylib::Init()
    {
        // Initialization code for Raylib rendering
        return true;
    }

    void RenderProviderRaylib::Shutdown()
    {
        // Shutdown code for Raylib rendering
    }

    void RenderProviderRaylib::BeginFrame()
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
    }

    void RenderProviderRaylib::EndFrame() { EndDrawing(); }
} // namespace Myriad
