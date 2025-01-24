#ifndef MYRIAD_RENDERING_RENDERER_H
#define MYRIAD_RENDERING_RENDERER_H

// #include "core/AllocatorService.h"
#include "asset/Font.h"
#include "asset/Texture2D.h"
#include "core/core.h"
#include "core/memory/MyrHandle.h"
#include "rendering/RenderProvider.h"

namespace Myriad
{
    class MYR_API Renderer : public RenderProvider
    {
      private:
        // AllocatorService allocator;
        RenderProvider *render_provider;

      public:
        Renderer();
        virtual ~Renderer();
        virtual bool Init() override;
        virtual bool Shutdown() override;

        virtual void ClearBackground(MyrColour colour) override;
        // Framebuffer Start Drawing
        virtual void BeginDrawing() override;
        virtual void EndDrawing() override;

        virtual void DrawCircle(Vector2 pos, float radius,
                                MyrColour colour) override;

        virtual void DrawTexture(Texture2D tex, Vector2 pos,
                                 MyrColour colour) override;
        virtual void DrawText(Font font, std::string text, Vector2 pos,
                              int size, MyrColour colour) override;
    };
} // namespace Myriad
#endif
