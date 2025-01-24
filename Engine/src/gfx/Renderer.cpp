#include "core/config.h"
#include "gfx/Renderer.h"
#include "gfx/IRenderer.h"
#include "gfx/IRendererProvider.h"
#include "io/MyrLogging.h"

#ifdef MYRIAD_RENDERER
#if MYRIAD_RENDERER == 1 // raylib
#include "gfx/RendererProviderRaylib.h"
#endif
#endif

namespace Myriad
{
  IRendererProvider::~IRendererProvider() {} // define ~IRendererProvider
  IRenderer::~IRenderer() {}                 // define ~IRenderer

  Renderer::Renderer()
  {
#ifdef MYRIAD_RENDERER
#if MYRIAD_RENDERER == RENDERER_RAYLIB // raylib
    p_renderer_provider_ = new RendererProviderRaylib();
#else
    MYR_CORE_ERROR("Invalid MYRIAD_RENDERER in config")
#endif
#else
    MYR_CORE_ERROR("MYRIAD_RENDERER not defined in config")
#endif
  }
  Renderer::~Renderer()
  {
    delete p_renderer_provider_;
  }
  void Renderer::ClearBackground(MyrColour colour)
  {
    p_renderer_provider_->ClearBackground(colour);
  }
  void Renderer::BeginDrawing()
  {
    p_renderer_provider_->BeginDrawing();
  }
  void Renderer::EndDrawing()
  {
    p_renderer_provider_->EndDrawing();
  }
  void Renderer::DrawCircle(Vector2 pos, float radius, MyrColour colour)
  {
    p_renderer_provider_->DrawCircle(pos, radius, colour);
  }
}
