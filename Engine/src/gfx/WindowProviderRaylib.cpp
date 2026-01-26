#include "gfx/WindowProviderRaylib.h"
#include "gfx/IWindow.h" //WindowState
#include "io/MyrLogging.h"
#if MYRIAD_RENDERER == RENDERER_RAYLIB

namespace Myriad
{

    WindowProviderRaylib::~WindowProviderRaylib()
    {
        MYR_CORE_TRACE("WindowProviderRaylib: destructor");
    }
    bool WindowProviderRaylib::Shutdown()
    {
        MYR_CORE_TRACE("Window Provider Raylib - shutdown");
        Close();
        return true;
    }

    bool WindowProviderRaylib::Init()
    {
        window_state_ = WindowState_t::CLOSED;
        return true;
    }
    bool WindowProviderRaylib::Init(int w, int h, const char *title)
    {
        // Raylib function, it doesn't return a value.
        MYR_CORE_INFO("Initing Raylib Renderer");
        ::InitWindow(w, h, title);
        if (::IsWindowReady()) // This tells us if the window inited.
        {
            window_state_ = WindowState_t::WINDOWED;
            return true;
        }
        MYR_CORE_ERROR("Unsuccessful.");
        return false;
    }
    void WindowProviderRaylib::SetFPS(int fps) { SetTargetFPS(fps); }

    bool WindowProviderRaylib::Close()
    {
        if (::IsWindowReady()) // only close if we opened
        {
            // raylib function, doesn't return a value
            ::CloseWindow();
            window_state_ = WindowState_t::CLOSED;
        }
        return true;
    }

    WindowState_t WindowProviderRaylib::GetWindowState()
    {
        // check if raylib thinks the window should close.
        if (::WindowShouldClose())
        {
            window_state_ = WindowState_t::CLOSING;
        }
        return window_state_;
    }
} // namespace Myriad
#endif
