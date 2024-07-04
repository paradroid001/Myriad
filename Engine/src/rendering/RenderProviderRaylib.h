#ifndef MYRIAD_RENDERING_RENDERPROVIDERRAYLIB_H
#define MYRIAD_RENDERING_RENDERPROVIDERRAYLIB_H

#include "RenderProvider.h"
#include "core/core.h"
#include "io/Log.h"
#include "raylib.h"

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

#endif
