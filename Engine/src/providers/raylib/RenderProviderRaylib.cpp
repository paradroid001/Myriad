#include "providers/raylib/RenderProviderRaylib.h"
#include "raylib.h"

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
        ::BeginDrawing();
        ::ClearBackground(RAYWHITE);
    }

    void RenderProviderRaylib::EndFrame() { ::EndDrawing(); }

    void RenderProviderRaylib::DrawText(AssetID_t font_id,
                                        const std::string &text, Vector2 pos,
                                        int size, Colour colour)
    {
        if (font_id == FONTHANDLE_INVALID)
        {
            // just use the default font.
            ::DrawText(text.c_str(), pos.x, pos.y, size,
                       (::Color){colour.r, colour.b, colour.g, colour.a});
        }
        else
        {
            // Unsupported just now.
            //::Font *fontptr = static_cast<::Font *>(font->GetDataPtr());
            //::DrawTextEx(*fontptr, text.c_str(), {pos.x, pos.y}, size, 1,
            //           (Color){colour.r, colour.b, colour.g, colour.a});
        }
    }
} // namespace Myriad
