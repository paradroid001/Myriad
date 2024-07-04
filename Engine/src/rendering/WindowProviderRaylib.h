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
        virtual bool Shutdown();
        virtual bool Init(int w, int h, const char *title);
        virtual void SetFPS(int fps);
        virtual bool ShouldClose();
        virtual void Close();
    };

    bool WindowProviderRaylib::Shutdown()
    {
        MYR_CORE_INFO("Raylib Window Provider shutting down");
        return true;
    }
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
