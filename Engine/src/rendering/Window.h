#ifndef MYRIAD_RENDERING_WINDOW_H
#define MYRIAD_RENDERING_WINDOW_H

#include "core/core.h"
#include "core/memory/Allocator.h"
#include "rendering/WindowProvider.h"

namespace Myriad
{
    class MYR_API Window : public WindowProvider
    {
      private:
        // MyrHandle<WindowProvider> *window_provider;

        WindowProvider *window_provider;

      public:
        Window(Allocator *allocator);
        ~Window();
        bool Init() override { return true; } // TODO this is useless.
        bool Init(int w, int h, const char *title) override;
        void SetFPS(int fps) override;
        bool ShouldClose() override;
        void Close() override;
        bool Shutdown() override;
    };
} // namespace Myriad

#endif
