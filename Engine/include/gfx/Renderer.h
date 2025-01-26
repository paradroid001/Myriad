#ifndef MYRIAD_GFX_RENDERER_H
#define MYRIAD_GFX_RENDERER_H

#include "core/core.h"
#include "gfx/IRenderer.h"
#include "gfx/IRendererProvider.h"
#include "asset/Texture2D.h"

namespace Myriad
{
  class MyrGameEngine; // fwd
  class AssetManager;  // fwd
  class MYR_API Renderer : public IRenderer, public IRenderFunctions
  {
  protected:
    IRendererProvider *p_renderer_provider_;
    MyrGameEngine *p_engine_;
    AssetManager *p_asset_manager_;

  public:
    Renderer();
    Renderer(MyrGameEngine *engine);
    virtual ~Renderer();
    virtual void ClearBackground(MyrColour colour) override;
    virtual void BeginDrawing() override;
    virtual void EndDrawing() override;

    virtual void DrawCircle(Vector2 pos, float radius,
                            MyrColour colour) override;
    virtual void DrawTexture(TexHandle_T tex, Vector2 pos,
                             MyrColour colour) override;
    virtual void DrawText(FontHandle_T fontid, std::string text, Vector2 pos,
                          int size, MyrColour colour) override;
  };
}

#endif
