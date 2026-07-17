#include "WindowProviderRaylib.h"

#include "raylib.h"

#include "io/Logging.h"

namespace Myriad
{
    // When we instantiate the window, it's closed.
    WindowProviderRaylib::WindowProviderRaylib()
        : window_state_(WindowState_t::CLOSED), window_props_(0)
    {
    }

    WindowProviderRaylib::~WindowProviderRaylib() {}

    bool WindowProviderRaylib::Init() { return true; }

    bool WindowProviderRaylib::Shutdown() { return true; }

    bool WindowProviderRaylib::Open(WindowConfig config, const char *title)
    {
        window_props_ = 0;
        unsigned int raylib_props = 0;
        if (config.resizable)
        {
            raylib_props |= ::FLAG_WINDOW_RESIZABLE;
        }
        if (config.vsync)
        {
            raylib_props |= ::FLAG_VSYNC_HINT;
        }
        if (config.fullscreen)
        {
            raylib_props |= ::FLAG_FULLSCREEN_MODE;
        }
        // TODO: this also implies maximise + not fullscreen
        if (config.borderless)
        {
            raylib_props |= ::FLAG_WINDOW_UNDECORATED;
        }
        // window_props_ = ???;
        ::SetConfigFlags(raylib_props);

        ::InitWindow(config.resolution.x, config.resolution.y, title);
        if (::IsWindowReady()) // This tells us if the window inited.
        {
            //::SetWindowState(::FLAG_WINDOW_RESIZABLE);
            window_state_ = WindowState_t::READY;
            return true;
        }
        MYR_CORE_ERROR(
            "WindowProviderRaylib::Open() - Failed to initialize window.");
        return false;
    }

    void WindowProviderRaylib::SetFPS(int fps)
    {

        // Calls the raylib function for target FPS
        ::SetTargetFPS(fps);
    }

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

    bool WindowProviderRaylib::WasResized() { return ::IsWindowResized(); }

    Vector2 WindowProviderRaylib::GetDimensions()
    {
        Vector2 retval;
        retval.x = ::GetScreenWidth();
        retval.y = ::GetScreenHeight();
        return retval;
    }

    WindowState_t WindowProviderRaylib::GetWindowState()
    {
        if (::WindowShouldClose())
        {
            window_state_ = WindowState_t::CLOSING;
        }
        // Otherwise return the state we know.
        return window_state_;
    }

    bool WindowProviderRaylib::WindowHasProp(WindowPropFlags_t prop_flag)
    {
        return false;
    }
    bool WindowProviderRaylib::WindowSetProp(WindowPropFlags_t prop_flag,
                                             bool val)
    {
        return false;
    }

    WindowProps_t WindowProviderRaylib::GetWindowProps()
    {
        // if (::IsWindowFullscreen())
        //{
        //     window_props_t &= WindowProps_t.
        // }
        //  check if raylib thinks the window should close.
        return window_props_;
    }

    bool WindowProviderRaylib::Minimise() { return false; }
    bool WindowProviderRaylib::Maximise() { return false; }
    bool WindowProviderRaylib::Restore() { return false; }
    bool WindowProviderRaylib::ToggleFullscreen() { return false; }
    bool WindowProviderRaylib::ToggleBorderlessWindowed() { return false; }

} // namespace Myriad
