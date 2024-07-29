#include "rendering/Window.h"
#include "core/memory/AllocatorProvider.h"
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
    Window::Window(AllocatorProvider *allocator)
    {
#if MYRIAD_RENDERER == RAYLIB
        uint16_t id = allocator->Alloc<WindowProviderRaylib>();
        window_provider = allocator->Get<WindowProviderRaylib>(id);
        //(MyrHandle<WindowProvider> *)(new MyrHandle<WindowProviderRaylib>(
        //    new WindowProviderRaylib()));
#endif
    }

    Window::~Window() {}

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
