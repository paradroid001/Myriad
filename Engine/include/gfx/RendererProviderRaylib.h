#ifndef MYRIAD_GFX_RENDERPROVIDERRAYLIB_H
#define MYRIAD_GFX_RENDERPROVIDERRAYLIB_H

#include "core/config.h"
#include "core/core.h"
#include "gfx/IRendererProvider.h"

#if MYRIAD_RENDERER == RENDERER_RAYLIB
#include "raylib.h"
namespace Myriad
{
  class RendererProviderRaylib : public IRendererProvider
  {
  public:
    RendererProviderRaylib();
    ~RendererProviderRaylib();

    // Interface Methods (IProvider)
    virtual bool Init() override;
    virtual bool Shutdown() override;

    // Interface Methods (IRenderer)
    virtual void ClearBackground(MyrColour colour) override;
    virtual void BeginDrawing() override;
    virtual void EndDrawing() override;

    virtual void DrawCircle(Vector2 pos, float radius,
                            MyrColour colour) override;
  };
}

#endif

#endif
