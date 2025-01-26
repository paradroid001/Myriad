#include "gfx/RendererProviderRaylib.h"
#include "asset/Texture2D.h"
#include "io/MyrLogging.h"
namespace Myriad
{
  RendererProviderRaylib::RendererProviderRaylib()
  {
    MYR_CORE_TRACE("RendererProviderRaylib constructor");
  }
  RendererProviderRaylib::~RendererProviderRaylib()
  {
    MYR_CORE_TRACE("RendererProviderRaylib destructor");
  }
  bool RendererProviderRaylib::Init()
  {
    return true;
  }
  bool RendererProviderRaylib::Shutdown()
  {
    return true;
  }
  void RendererProviderRaylib::ClearBackground(MyrColour colour)
  {
    // raylib function
    ::ClearBackground({colour.r, colour.g, colour.b, colour.a});
  }
  void RendererProviderRaylib::BeginDrawing()
  {
    ::BeginDrawing();
  }
  void RendererProviderRaylib::EndDrawing()
  {
    ::EndDrawing();
  }

  void Myriad::RendererProviderRaylib::DrawCircle(Vector2 pos, float radius,
                                                  MyrColour colour)
  {
    ::DrawCircle((int)pos.x, (int)pos.y, radius,
                 {colour.r, colour.g, colour.b, colour.a});
  }

  void Myriad::RendererProviderRaylib::DrawTexture(Texture2D tex, Vector2 pos, MyrColour colour)
  {
    // I can make this cast because I know tex is a raylib texture object.
    ::Texture2D t = *(static_cast<::Texture2D *>(tex.GetTexPtr()));
    ::DrawTexture(t, pos.x, pos.y, {colour.r, colour.g, colour.b, colour.a});
  }

  void Myriad::RendererProviderRaylib::DrawText(Font font, std::string text, Vector2 pos, int size, MyrColour colour)
  {
    ::Font raylib_font = *(static_cast<::Font *>(font.GetFontPtr()));
    ::DrawTextPro(raylib_font, text.c_str(), {pos.x, pos.y}, {0, 0}, 0.0f, size, 1.0f, {colour.r, colour.g, colour.b, colour.a});
  }

}
