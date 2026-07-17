#include "myriad.h"

#include "gfx/IWindowProvider.h"
#include "gfx/Window.h"
#include "gfx/WindowProviderRaylib.h"

namespace Myriad
{
    // A convenient place to define the pure virtual destructor for
    // IWindow
    // and IWindowProvider
    IWindow::~IWindow() {}

    IWindowProvider::~IWindowProvider() {}

    Window::Window()
    {
        // If the engine config is raylib,
        // then provide this.
        window_provider_ = new WindowProviderRaylib();
        if (window_provider_ != nullptr)
        {
            window_provider_->Init();
        }
    }
    Window::~Window()
    {
        if (window_provider_ != nullptr)
        {
            window_provider_->Shutdown();
            delete window_provider_;
            window_provider_ = nullptr;
        }
    }

    bool Window::Open(WindowConfig config, const char *title)
    {
        if (window_provider_ != nullptr)
        {
            return window_provider_->Open(config, title);
        }
        return false;
    }
    void Window::SetFPS(int fps)
    {
        if (window_provider_ != nullptr)
        {
            window_provider_->SetFPS(fps);
        }
    }

    bool Window::Close()
    {
        if (window_provider_ != nullptr)
        {
            return window_provider_->Close();
        }
        return false;
    }

    WindowState_t Window::GetWindowState()
    {
        if (window_provider_ != nullptr)
        {
            return window_provider_->GetWindowState();
        }
        return WindowState_t::CLOSED;
    }
    bool Window::WasResized()
    {
        if (window_provider_ != nullptr)
        {
            return window_provider_->WasResized();
        }
        return false;
    }
    Vector2 Window::GetDimensions()
    {
        if (window_provider_ != nullptr)
        {
            return window_provider_->GetDimensions();
        }
        return {0, 0};
    }
    WindowProps_t Window::GetWindowProps()
    {
        if (window_provider_ != nullptr)
        {
            return window_provider_->GetWindowProps();
        }
        return 0;
    }

    bool Window::WindowHasProp(WindowPropFlags_t prop_flag) { return false; }
    bool Window::WindowSetProp(WindowPropFlags_t prop_flag, bool val)
    {
        return false;
    }
    bool Window::Minimise() { return false; }
    bool Window::Maximise() { return false; }
    bool Window::Restore() { return false; }
    bool Window::ToggleFullscreen() { return false; }
    bool Window::ToggleBorderlessWindowed() { return false; }
} // namespace Myriad
