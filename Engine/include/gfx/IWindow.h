#ifndef MYRIAD_GFX_IWINDOW_H
#define MYRIAD_GFX_IWINDOW_H

#include "core/core.h"

namespace Myriad
{
  typedef enum
  {
    OPENING,
    MINIMISED,  // minimised window
    WINDOWED,   // a window, but not max or min
    MAXIMISED,  // maximised window
    FULLSCREEN, // fullscreen mode
    CLOSING,
    CLOSED
  } WindowState_t;

  // An interface for windows
  class MYR_API IWindow
  {
  public:
    virtual ~IWindow() = 0;
    virtual bool Init(int w, int h, const char *title) = 0;
    virtual void SetFPS(int fps) = 0;
    virtual bool Close() = 0;
    WindowState_t virtual GetWindowState() = 0;
  };
}

#endif
