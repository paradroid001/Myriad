#ifndef MYRIAD_GFX_IRENDERPROVIDER_H
#define MYRIAD_GFX_IRENDERPROVIDER_H

#include "core/core.h"
#include "core/IProvider.h"
#include "gfx/IRenderer.h"

namespace Myriad
{
  class MYR_API IRendererProvider : public IRenderer, public IProvider, public IRenderFunctionsInternal
  {
  public:
    virtual ~IRendererProvider() = 0;

    // Interface Methods (IProvider)
    virtual bool Init() = 0;
    virtual bool Shutdown() = 0;

    // Interface Methods (IRenderer)
    virtual void ClearBackground(MyrColour colour) = 0;
    virtual void BeginDrawing() = 0;
    virtual void EndDrawing() = 0;

    virtual void DrawCircle(Vector2 pos, float radius,
                            MyrColour colour) = 0;
    virtual void DrawTexture(Texture2D tex, Vector2 pos,
                             MyrColour colour) = 0;
    virtual void DrawText(Font font, std::string text, Vector2 pos,
                          int size, MyrColour colour) = 0;
  };
}

#endif
