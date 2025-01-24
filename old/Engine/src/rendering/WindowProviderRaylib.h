#ifndef MYRIAD_RENDERING_WINDOWPROVIDERRAYLIB_H
#define MYRIAD_RENDERING_WINDOWPROVIDERRAYLIB_H

#include "WindowProvider.h"
#include "core/core.h"
#include "io/Log.h"
#include "raylib.h"

namespace Myriad
{
    class MYR_API WindowProviderRaylib : public WindowProvider
    {
      public:
        virtual bool Shutdown() override;
        virtual bool Init();
        virtual bool Init(int w, int h, const char *title) override;
        virtual void SetFPS(int fps) override;
        virtual bool ShouldClose() override;
        virtual void Close() override;
    };

    bool WindowProviderRaylib::Shutdown()
    {
        MYR_CORE_INFO("Raylib Window Provider shutting down");
        return true;
    }

    // TODO this is included because it has to be, it's an abstract method.
    bool WindowProviderRaylib::Init() { return true; }
    bool WindowProviderRaylib::Init(int w, int h, const char *title)
    {
        // Raylib Function
        InitWindow(w, h, title);
        return true;
    }
    void WindowProviderRaylib::SetFPS(int fps) { SetTargetFPS(fps); }
    bool WindowProviderRaylib::ShouldClose() { return WindowShouldClose(); }
    void WindowProviderRaylib::Close() { CloseWindow(); }
} // namespace Myriad

#endif
