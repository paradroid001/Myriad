#include "gfx/IWindow.h" //WindowState
#include "gfx/WindowProviderRaylib.h"
#include "io/MyrLogging.h"
#if MYRIAD_RENDERER == RENDERER_RAYLIB

namespace Myriad
{
  bool WindowProviderRaylib::Shutdown() { return true; }
  bool WindowProviderRaylib::Init()
  {
    window_state_ = WindowState_t::CLOSED;
    return true;
  }
  bool WindowProviderRaylib::Init(int w, int h, const char *title)
  {
    // Raylib function, it doesn't return a value.
    InitWindow(w, h, title);
    window_state_ = WindowState_t::WINDOWED;
    return true;
  }
  void WindowProviderRaylib::SetFPS(int fps) { SetTargetFPS(fps); }

  bool WindowProviderRaylib::Close()
  {
    // raylib function, doesn't return a value
    ::CloseWindow();
    window_state_ = WindowState_t::CLOSED;
    return true;
  }

  WindowState_t WindowProviderRaylib::GetWindowState()
  {
    // check if raylib thinks the window should close.
    if (::WindowShouldClose())
    {
      window_state_ = WindowState_t::CLOSING;
    }
    return window_state_;
  }
}
#endif
