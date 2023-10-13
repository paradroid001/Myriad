#include "Window.h"

#ifdef MYRIAD_INTERNAL
    #include "MyriadConfig.h"
#else
// fwd declare the functions we use.
void InitWindow(int width, int height, const char *title);
void SetTargetFPS(int fps);
bool WindowShouldClose();
void CloseWindow();
#endif
// #include "raylib.h"

namespace Myriad
{
    Window::Window() {}
    Window::~Window() {}
    void Window::Init(int w, int h, const char *title)
    {
#if MYRIAD_RENDERER == RAYLIB
        InitWindow(w, h, title);
#endif
    }

    void Window::SetFPS(int fps)
    {
#if MYRIAD_RENDERER == RAYLIB
        SetTargetFPS(fps);
    }
#endif
    bool Window::ShouldClose()
    {
#if MYRIAD_RENDERER == RAYLIB
        return WindowShouldClose();
#endif
    }
    void Window::Close()
    {
#if MYRIAD_RENDERER == RAYLIB
        CloseWindow();
#endif
    }
} // namespace Myriad