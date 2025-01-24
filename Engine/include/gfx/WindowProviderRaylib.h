#ifndef MYRIAD_WINDOW_PROVIDER_RAYLIB_H
#define MYRIAD_WINDOW_PROVIDER_RAYLIB_H

#include "core/config.h"
#include "core/core.h"
#include "gfx/IWindowProvider.h"

#if MYRIAD_RENDERER == RENDERER_RAYLIB
#include "raylib.h"
#endif

namespace Myriad
{
  class WindowProviderRaylib : public IWindowProvider
  {
  protected:
    WindowState_t window_state_;

  public:
    // IWindowProvider
    bool Init() override;
    bool Shutdown() override;
    bool Init(int w, int h, const char *title) override;
    void SetFPS(int fps) override;
    bool Close() override;
    WindowState_t GetWindowState() override;
  };
}
#endif
