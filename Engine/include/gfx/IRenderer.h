#ifndef MYRIAD_GFX_IRENDERER_H
#define MYRIAD_GFX_IRENDERER_H

#include "core/core.h"
#include "asset/Texture2D.h"
#include "asset/Font.h"

namespace Myriad
{
  class MYR_API IRenderer
  {
  public:
    virtual ~IRenderer() = 0;
    virtual void ClearBackground(MyrColour colour) = 0;
    virtual void BeginDrawing() = 0;
    virtual void EndDrawing() = 0;
  };
  class MYR_API IRenderFunctions
  {
    virtual void DrawCircle(Vector2 pos, float radius,
                            MyrColour colour) = 0;

    virtual void DrawTexture(TexHandle_T texid, Vector2 pos,
                             MyrColour colour) = 0;
    virtual void DrawText(FontHandle_T fontid, std::string text, Vector2 pos,
                          int size, MyrColour colour) = 0;
  };
  class MYR_API IRenderFunctionsInternal
  {
    virtual void DrawCircle(Vector2 pos, float radius,
                            MyrColour colour) = 0;

    virtual void DrawTexture(Texture2D tex, Vector2 pos,
                             MyrColour colour) = 0;
    virtual void DrawText(Font font, std::string text, Vector2 pos,
                          int size, MyrColour colour) = 0;
  };
}

#endif
