#include "rendering/Window.h"
#include "core/memory/Allocator.h"

#include "core/memory/MyrHandle.h"

// #include "core/MyrHandle.h"
#ifdef MYRIAD_INTERNAL
    #include "core/MyriadConfig.h"
#else

// fwd declare the functions we use.
// void InitWindow(int width, int height, const char *title);
// void SetTargetFPS(int fps);
// bool WindowShouldClose();
// void CloseWindow();
#endif

#if MYRIAD_RENDERER == RAYLIB
    #include "rendering/WindowProviderRaylib.h"
#endif

namespace Myriad
{
    Window::Window(Allocator *allocator) : WindowProvider()
    {
#if MYRIAD_RENDERER == RAYLIB
        // MyrHandle<WindowProviderRaylib> handle =
        //     allocator->Alloc<WindowProviderRaylib>();
        // window_provider = handle.Get();

        window_provider = new WindowProviderRaylib();

        //->Get<WindowProviderRaylib>(id);
        //(MyrHandle<WindowProvider> *)(new MyrHandle<WindowProviderRaylib>(
        //    new WindowProviderRaylib()));
#endif
    }

    Window::~Window() { delete window_provider; }

    bool Window::Init(int w, int h, const char *title)
    {
        return window_provider->Init(w, h, title);
    }

    void Window::SetFPS(int fps) { window_provider->SetFPS(fps); }

    bool Window::ShouldClose() { return window_provider->ShouldClose(); }

    void Window::Close() { window_provider->Close(); }

    bool Window::Shutdown()
    {
        MYR_CORE_INFO("Window Shutting Down");
        return true;
    };
} // namespace Myriad
