#ifndef MYRIAD_RENDERING_WINDOW_H
#define MYRIAD_RENDERING_WINDOW_H

#include "core/MyrHandle.h"
#include "core/core.h"
#include "rendering/WindowProvider.h"

namespace Myriad
{
    class MYR_API Window : public WindowProvider
    {
      private:
        MyrHandle<WindowProvider> *window_provider;

      public:
        Window();
        ~Window();
        bool Init(int w, int h, const char *title);
        void SetFPS(int fps);
        bool ShouldClose();
        void Close();
        bool Shutdown();
    };
} // namespace Myriad

#endif
