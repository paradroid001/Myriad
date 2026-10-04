#include "providers/raylib/WindowProviderRaylib.h"
#include "io/Logging.h"

namespace Myriad
{
    bool WindowProviderRaylib::Open(const WindowConfig &config)
    {
        ::InitWindow((int)config.resolution.x, (int)config.resolution.y,
                     config.title.c_str());
        if (::IsWindowReady()) // This tells us if the window inited.
        {
            state_ = WindowState_t::READY;
            return true;
        }
        MYR_CORE_ERROR(
            "WindowProviderRaylib::Open() - Failed to initialize window.");
        state_ = WindowState_t::CLOSED;
        return false;
    }

    bool WindowProviderRaylib::Init()
    {
        // raylib doesn't have a separate init function for the window,
        // so we just check if the window is ready
        return ::IsWindowReady();
    }

    void WindowProviderRaylib::Close()
    {
        if (::IsWindowReady()) // only close if we opened
        {
            // raylib function, doesn't return a value
            ::CloseWindow();
            state_ = WindowState_t::CLOSED;
        }
    }

    void WindowProviderRaylib::Shutdown() { Close(); }

    WindowState_t WindowProviderRaylib::GetState()
    {
        if (::WindowShouldClose())
        {
            state_ = WindowState_t::CLOSING;
        }
        else if (!::IsWindowReady())
        {
            state_ = WindowState_t::CLOSED;
        }
        return state_;
    }
} // namespace Myriad
