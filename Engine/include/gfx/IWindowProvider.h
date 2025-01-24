#ifndef MYRIAD_GFX_IWINDOW_PROVIDER_H
#define MYRIAD_GFX_IWINDOW_PROVIDER_H

#include "core/core.h"
#include "core/IProvider.h"
#include "gfx/IWindow.h"

namespace Myriad
{
  class MYR_API IWindowProvider : public IWindow, public IProvider
  {
  public:
    virtual ~IWindowProvider() = 0;

    // Interface Methods (IProvider)
    virtual bool Init() = 0;
    virtual bool Shutdown() = 0;

    // Interface Methods (IWindow)
    virtual bool Init(int w, int h, const char *title) = 0;
    virtual void SetFPS(int fps) = 0;
    virtual WindowState_t GetWindowState() = 0;
    virtual bool Close() = 0;
  };
}

#endif
