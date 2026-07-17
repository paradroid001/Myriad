#include <memory> //make_shared

#include "io/Logging.h"
#include "myriad.h" //Texture2D

#include "gfx/RendererProviderRaylib.h"

#include "asset/Texture2DProviderRaylib.h" //Texture2DProviderRaylin

namespace Myriad
{
    RendererProviderRaylib::RendererProviderRaylib(
        std::shared_ptr<Texture2DProviderRaylib> tex_provider)
    {
        // NOTE: We assume if you're using the raylib
        //       renderer provider, you'd be passed a
        //       raylib texture provider.
        texture2d_provider_ = tex_provider;
        MYR_CORE_TRACE("Render Provider Raylib constructed");
    }
    RendererProviderRaylib::~RendererProviderRaylib()
    {
        MYR_CORE_TRACE("Render Provider Raylib destroyed");
    }

    bool RendererProviderRaylib::Init() { return true; }

    bool RendererProviderRaylib::Shutdown() { return true; }

    void RendererProviderRaylib::ClearBackground(MyrColour colour)
    {
        ::ClearBackground({colour.r, colour.g, colour.b, colour.a});
    }

    void RendererProviderRaylib::BeginFrame() { ::BeginDrawing(); }

    void RendererProviderRaylib::EndFrame() { ::EndDrawing(); }

    void RendererProviderRaylib::DrawRectangle(Vector2 pos, Vector2 size,
                                               MyrColour colour)
    {
        ::DrawRectangle(pos.x, pos.y, size.x, size.y,
                        (Color){colour.r, colour.g, colour.b, colour.a});
    }

    void RendererProviderRaylib::DrawCircle(Vector2 pos, float radius,
                                            MyrColour colour)
    {
        ::DrawCircle(pos.x, pos.y, radius,
                     (Color){colour.r, colour.g, colour.b, colour.a});
    }

    void RendererProviderRaylib::DrawTexture(AssetID_t tex_asset, Vector2 pos,
                                             MyrColour colour)
    {
        ::Texture2D *texptr = static_cast<::Texture2D *>(
            texture2d_provider_->GetTexPtr(tex_asset));
        if (texptr == nullptr)
        {
            MYR_CORE_ERROR("Attempted to draw an invalid asset id");
            return;
        }
        ::DrawTexture(*texptr, pos.x, pos.y,
                      (Color){colour.r, colour.b, colour.g, colour.a});
    }

    void RendererProviderRaylib::DrawTexture(AssetID_t tex_asset, Rect2D rect,
                                             Vector2 pos, MyrColour colour)
    {
        ::Texture2D *texptr = static_cast<::Texture2D *>(
            texture2d_provider_->GetTexPtr(tex_asset));
        if (texptr == nullptr)
        {
            MYR_CORE_ERROR("Attempted to draw an invalid asset id");
            return;
        }
        // Draw texture using top left coords plus width and height. No zoom.
        // Assume anchor is at top left.
        ::DrawTexturePro(*texptr,
                         {rect.Left(), rect.Top(), rect.size.x, rect.size.y},
                         {pos.x, pos.y, rect.size.x, rect.size.y}, {0, 0}, 0.0f,
                         (Color){colour.r, colour.b, colour.g, colour.a});
    }

    void RendererProviderRaylib::DrawText(Asset *font, std::string text,
                                          Vector2 pos, int size,
                                          MyrColour colour)
    {
        if (font == nullptr)
        {
            MYR_CORE_ERROR("Attempted to draw text with a null font");
            return;
        }
        ::Font *fontptr = static_cast<::Font *>(font->GetDataPtr());
        ::DrawTextEx(*fontptr, text.c_str(), {pos.x, pos.y}, size, 1,
                     (Color){colour.r, colour.b, colour.g, colour.a});
    }
} // namespace Myriad
