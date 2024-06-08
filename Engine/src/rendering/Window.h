#ifndef MYRIAD_RENDERING_WINDOW_H
#define MYRIAD_RENDERING_WINDOW_H

#include "core/core.h"

namespace Myriad
{
    class MYR_API Window
    {
      public:
        Window();
        ~Window();
        void Init(int w, int h, const char *title);
        void SetFPS(int fps);
        bool ShouldClose();
        void Close();
    };
} // namespace Myriad

#endif
