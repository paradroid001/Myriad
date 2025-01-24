#ifndef MYRIAD_GFX_RENDERER_H
#define MYRIAD_GFX_RENDERER_H

#include "core/core.h"
#include "gfx/IRenderer.h"
#include "gfx/IRendererProvider.h"

namespace Myriad
{
  class MYR_API Renderer : public IRenderer
  {
  protected:
    IRendererProvider *p_renderer_provider_;

  public:
    Renderer();
    virtual ~Renderer();
    virtual void ClearBackground(MyrColour colour) override;
    virtual void BeginDrawing() override;
    virtual void EndDrawing() override;

    virtual void DrawCircle(Vector2 pos, float radius,
                            MyrColour colour) override;
  };
}

#endif
