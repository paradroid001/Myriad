#include "gfx/Renderer.h"
#include "asset/AssetManager.h" //to access assets from ids
#include "asset/Font.h"
#include "asset/Texture2D.h"
#include "core/MyrGameEngine.h" //to ref back tot GE
#include "core/config.h"
#include "gfx/IRenderer.h"
#include "gfx/IRendererProvider.h"
#include "io/MyrLogging.h"

#include "core/MyrProfiling.h"

#ifdef MYRIAD_RENDERER
    #if MYRIAD_RENDERER == 1 // raylib
        #include "gfx/RendererProviderRaylib.h"
    #endif
#endif

namespace Myriad
{
    // Abstract virtual destructors should be defined outside the class
    // This is a convenient place to do it, rather than creating more cpp files.
    IRendererProvider::~IRendererProvider() {} // define ~IRendererProvider
    IRenderer::~IRenderer() {}                 // define ~IRenderer

    Renderer::Renderer()
    {
#ifdef MYRIAD_RENDERER
    #if MYRIAD_RENDERER == RENDERER_RAYLIB // raylib
        p_renderer_provider_ = new RendererProviderRaylib();
    #else
        MYR_CORE_ERROR("Invalid MYRIAD_RENDERER in config")
    #endif
#else
        MYR_CORE_ERROR("MYRIAD_RENDERER not defined in config")
#endif
    }

    Renderer::Renderer(MyrGameEngine *engine) : Renderer()
    {
        p_engine_ = engine;
        p_asset_manager_ = &(p_engine_->GetAssetManager());
    }

    Renderer::~Renderer() { delete p_renderer_provider_; }

    void Renderer::ClearBackground(MyrColour colour)
    {
        MyrProfileScoped(Renderer__ClearBackground);
        p_renderer_provider_->ClearBackground(colour);
    }
    void Renderer::BeginDrawing()
    {
        MyrProfileScoped(Renderer__BeginDrawing);
        p_renderer_provider_->BeginDrawing();
    }
    void Renderer::EndDrawing()
    {
        MyrProfileScoped(Renderer__EndDrawing);
        p_renderer_provider_->EndDrawing();
    }
    void Renderer::DrawCircle(Vector2 pos, float radius, MyrColour colour)
    {
        MyrProfileScoped(Renderer__Circle);
        p_renderer_provider_->DrawCircle(pos, radius, colour);
    }
    void Renderer::DrawTexture(TexHandle_T h_tex, Vector2 pos, MyrColour colour)
    {
        MyrProfileScoped(Renderer__DrawTexture);
        Texture2D *p_tex = p_asset_manager_->GetTexturePointer(h_tex);
        if (p_tex != nullptr)
            p_renderer_provider_->DrawTexture(*p_tex, pos, colour);
        else
            MYR_CORE_ERROR("Retrieved texture was null");
    }

    void Renderer::DrawText(FontHandle_T h_font, std::string text, Vector2 pos,
                            int size, MyrColour colour)
    {
        MyrProfileScoped(Renderer__DrawText);

        Font *p_font = p_asset_manager_->GetFontPointer(h_font);
        if (p_font != nullptr)
        {
            p_renderer_provider_->DrawText(*p_font, text, pos, size, colour);
        }
        else
            MYR_CORE_ERROR("Retrieved font was null");
    }
} // namespace Myriad
