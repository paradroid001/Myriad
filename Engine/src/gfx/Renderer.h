#ifndef _MYRIAD_RENDERER_H_
#define _MYRIAD_RENDERER_H_
#include <memory>

#include "myriad.h"

#include "gfx/IRenderProvider.h"

namespace Myriad
{
    // class MyrGameEngine; // fwd
    // class AssetManager;  // fwd

    class Renderer : public IRenderer
    {
      protected:
        std::unique_ptr<IRendererProvider> renderer_provider_;

      public:
        Renderer(std::shared_ptr<AssetManager> sp_asset_manager);
        virtual ~Renderer();
        virtual void ClearBackground(MyrColour colour) override;
        virtual void BeginFrame() override;
        virtual void EndFrame() override;

        virtual void DrawRectangle(Vector2 pos, Vector2 size,
                                   MyrColour colour) override;
        virtual void DrawCircle(Vector2 pos, float radius,
                                MyrColour colour) override;

        virtual void DrawTexture(AssetID_t tex_asset, Vector2 pos,
                                 MyrColour colour) override;
        virtual void DrawTexture(AssetID_t tex_asset, Rect2D rect, Vector2 pos,
                                 MyrColour colour) override;
        virtual void DrawText(Asset *font, std::string text, Vector2 pos,
                              int size, MyrColour colour) override;
    };
} // namespace Myriad
#endif
