#ifndef MYRIAD_RENDERING_RENDERPROVIDERRAYLIB_H
#define MYRIAD_RENDERING_RENDERPROVIDERRAYLIB_H

#include "core/core.h"
#include "io/Log.h"
#include "raylib.h"
#include "rendering/RenderProvider.h"

#include "asset/TextureProviderRaylib.h"

namespace Myriad
{
    class MYR_API RenderProviderRaylib : public RenderProvider
    {
      public:
        virtual bool Shutdown() override;
        virtual bool Init() override;
        virtual void ClearBackground(MyrColour colour) override;
        virtual void BeginDrawing() override;
        virtual void EndDrawing() override;
        virtual void DrawCircle(Vector2 pos, float radius,
                                MyrColour colour) override;
        virtual void DrawTexture(Texture2D tex, Vector2 pos, MyrColour colour);
        virtual void DrawText(Font font, std::string text, Vector2 pos,
                              int size, MyrColour colour);
    };
} // namespace Myriad

bool Myriad::RenderProviderRaylib::Shutdown() { return true; }
bool Myriad::RenderProviderRaylib::Init() { return true; }
void Myriad::RenderProviderRaylib::ClearBackground(MyrColour colour)
{
    ::ClearBackground({colour.r, colour.g, colour.b, colour.a});
}
void Myriad::RenderProviderRaylib::BeginDrawing() { ::BeginDrawing(); }
void Myriad::RenderProviderRaylib::EndDrawing() { ::EndDrawing(); }

void Myriad::RenderProviderRaylib::DrawCircle(Vector2 pos, float radius,
                                              MyrColour colour)
{
    ::DrawCircle((int)pos.x, (int)pos.y, radius,
                 {colour.r, colour.g, colour.b, colour.a});
}

void Myriad::RenderProviderRaylib::DrawTexture(Texture2D tex, Vector2 pos,
                                               MyrColour colour)
{
    // I can make this cast because I know tex is a raylib texture object.
    ::Texture2D t = *(static_cast<::Texture2D *>(tex.GetTexPtr()));
    ::DrawTexture(t, pos.x, pos.y, {colour.r, colour.g, colour.b, colour.a});
}
void Myriad::RenderProviderRaylib::DrawText(Font font, std::string text,
                                            Vector2 pos, int size,
                                            MyrColour colour)
{
    // I could get the font but let's just do a default draw
    ::DrawText(text.c_str(), pos.x, pos.y, size,
               {colour.r, colour.g, colour.b, colour.a});
}

#endif
