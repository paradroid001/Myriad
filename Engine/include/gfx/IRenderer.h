#ifndef MYRIAD_GFX_IRENDERER_H
#define MYRIAD_GFX_IRENDERER_H

#include "core/core.h"

namespace Myriad
{
  class MYR_API IRenderer
  {
  public:
    virtual ~IRenderer() = 0;
    virtual void ClearBackground(MyrColour colour) = 0;
    virtual void BeginDrawing() = 0;
    virtual void EndDrawing() = 0;

    virtual void DrawCircle(Vector2 pos, float radius,
                            MyrColour colour) = 0;

    // virtual void DrawTexture(Texture2D tex, Vector2 pos,
    //                           MyrColour colour) =0;
    // virtual void DrawText(Font font, std::string text, Vector2 pos,
    //                       int size, MyrColour colour) = 0;
  };
}

#endif
