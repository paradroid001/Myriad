#ifndef __WINDOW_H_
#define __WINDOW_H_

#include "core.h"

namespace myriad
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
} // namespace myriad

#endif
