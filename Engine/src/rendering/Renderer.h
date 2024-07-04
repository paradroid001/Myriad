#ifndef MYRIAD_RENDERING_RENDERER_H
#define MYRIAD_RENDERING_RENDERER_H

// #include "core/AllocatorService.h"
#include "core/MyrHandle.h"
#include "core/core.h"
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
    };
} // namespace Myriad
#endif
