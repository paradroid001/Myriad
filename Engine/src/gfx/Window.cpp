#include "gfx/Window.h"
#include "core/config.h"
#include "io/MyrLogging.h"
#include <memory>

#if MYRIAD_RENDERER == RENDERER_RAYLIB
    #include "gfx/WindowProviderRaylib.h"
#endif

namespace Myriad
{
    // Declaring destructors for various things.
    IWindow::~IWindow() {}
    IWindowProvider::~IWindowProvider() {}

    Window::~Window()
    {
        // Shutdown the provider - make sure it has freed all memory.
        p_window_provider_->Shutdown();

        if (p_window_provider_ != nullptr)
            delete p_window_provider_;
        MYR_CORE_TRACE("Window destructor!");
    }
    bool Window::Init(int w, int h, const char *title)
    {
#if MYRIAD_RENDERER == RENDERER_RAYLIB
        p_window_provider_ = new WindowProviderRaylib();
#else
        MYR_CORE_ERROR("Cannot initialise window: unknown renderer");
#endif
        if (p_window_provider_)
        {
            return p_window_provider_->Init(w, h, title);
        }
        return false;
    }
    bool Window::Close() { return p_window_provider_->Close(); }
    void Window::SetFPS(int fps) { p_window_provider_->SetFPS(fps); }
    WindowState_t Window::GetWindowState()
    {
        return p_window_provider_->GetWindowState();
    }
} // namespace Myriad
