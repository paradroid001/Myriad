#ifndef MYRIAD_RENDERING_WINDOWPROVIDER_H
#define MYRIAD_RENDERING_WINDOWPROVIDER_H

#include "core/Provider.h"

namespace Myriad
{
    class WindowProvider : public Provider
    {
      public:
        virtual ~WindowProvider();

        // Interface Methods (Provider)
        virtual bool Shutdown() = 0;

        // Interface Methods (WindowProvider)
        virtual bool Init(int w, int h, const char *title) = 0;
        virtual void SetFPS(int fps) = 0;
        virtual bool ShouldClose() = 0;
        virtual void Close() = 0;
    };
} // namespace Myriad

#endif
