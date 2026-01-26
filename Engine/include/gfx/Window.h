#ifndef MYRIAD_GFX_WINDOW_H
#define MYRIAD_GFX_WINDOW_H

#include "core/core.h"
#include "gfx/IWindow.h"
#include "gfx/IWindowProvider.h"
#include <memory>

namespace Myriad
{
    class MYR_API Window : public IWindow
    {
      protected:
        IWindowProvider *p_window_provider_;

      public:
        virtual ~Window();
        virtual bool Init(int w, int h, const char *title) override;
        virtual void SetFPS(int fps) override;
        virtual bool Close() override;
        WindowState_t virtual GetWindowState() override;
    };

} // namespace Myriad

#endif
