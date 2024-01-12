#ifndef __WINDOW_H_
#define __WINDOW_H_

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
