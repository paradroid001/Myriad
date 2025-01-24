#include "gfx/RendererProviderRaylib.h"
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

}
