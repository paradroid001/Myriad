#include <memory>

#include "myriad.h"

#include "gfx/Renderer.h"
#include "gfx/RendererProviderRaylib.h"

namespace Myriad
{
    IRenderer::~IRenderer() {} // define ~IRenderer

    Renderer::Renderer(std::shared_ptr<AssetManager> sp_asset_manager)
        : IRenderer()
    {
        // If the engine config is raylib,
        // then provide this.
        // Have to assume a Texture2DProviderRaylib is going to
        // have been made if a RenderProviderRaylib is being made.
        renderer_provider_ = std::make_unique<RendererProviderRaylib>(
            std::static_pointer_cast<Texture2DProviderRaylib>(
                sp_asset_manager->GetTextureProvider()));
    }

    Renderer::~Renderer()
    {
        // Clean up the provider if it exists
        if (renderer_provider_)
        {
            renderer_provider_->Shutdown();
            renderer_provider_.reset();
        }
    }

    void Renderer::ClearBackground(MyrColour colour)
    {
        if (renderer_provider_)
            renderer_provider_->ClearBackground(colour);
    }

    void Renderer::BeginFrame()
    {
        if (renderer_provider_)
            renderer_provider_->BeginFrame();
    }

    void Renderer::EndFrame()
    {
        if (renderer_provider_)
            renderer_provider_->EndFrame();
    }

    void Renderer::DrawRectangle(Vector2 pos, Vector2 size, MyrColour colour)
    {
        if (renderer_provider_)
            renderer_provider_->DrawRectangle(pos, size, colour);
    }

    void Renderer::DrawCircle(Vector2 pos, float radius, MyrColour colour)
    {
        if (renderer_provider_)
            renderer_provider_->DrawCircle(pos, radius, colour);
    }

    void Renderer::DrawTexture(AssetID_t tex_asset, Vector2 pos,
                               MyrColour colour)
    {
        if (renderer_provider_)
            renderer_provider_->DrawTexture(tex_asset, pos, colour);
    }

    void Renderer::DrawTexture(AssetID_t tex_asset, Rect2D rect, Vector2 pos,
                               MyrColour colour)
    {
        if (renderer_provider_)
            renderer_provider_->DrawTexture(tex_asset, rect, pos, colour);
    }

    void Renderer::DrawText(Asset *font, std::string text, Vector2 pos,
                            int size, MyrColour colour)
    {
        if (renderer_provider_)
            renderer_provider_->DrawText(font, text, pos, size, colour);
    }
} // namespace Myriad
