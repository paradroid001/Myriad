#include "rendering/Renderer.h"
#include "asset/Texture2D.h"

#ifdef MYRIAD_INTERNAL
    #include "core/MyriadConfig.h"
#else
// you can forward declare functions and classes here.
#endif

#if MYRIAD_RENDERER == RAYLIB
    #include "rendering/RenderProviderRaylib.h"
// Other render providers can go here.
#endif

namespace Myriad
{
    Renderer::Renderer() {}
    Renderer::~Renderer() {}

    bool Renderer::Init()
    {
        // allocator.Init();
#if MYRIAD_RENDERER == RAYLIB
        render_provider =
            static_cast<RenderProvider *>(new RenderProviderRaylib());
#endif
        return true;
    }

    bool Renderer::Shutdown()
    {
        // allocator.Shutdown();
        if (render_provider != nullptr)
        {
            delete render_provider;
            render_provider = nullptr;
        }
        return true;
    }

    void Renderer::ClearBackground(MyrColour colour)
    {
        render_provider->ClearBackground(colour);
    }
    void Renderer::BeginDrawing() { render_provider->BeginDrawing(); }
    void Renderer::EndDrawing() { render_provider->EndDrawing(); }
    void Renderer::DrawCircle(Vector2 pos, float radius, MyrColour colour)
    {
        render_provider->DrawCircle(pos, radius, colour);
    }
    void Renderer::DrawTexture(Texture2D tex, Vector2 pos, MyrColour colour)
    {
        render_provider->DrawTexture(tex, pos, colour);
    }
} // namespace Myriad
