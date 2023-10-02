#include "Window.h"
#include "raylib.h"

namespace myriad
{
    Window::Window() {}
    Window::~Window() {}
    void Window::Init(int w, int h, const char *title)
    {
        InitWindow(w, h, title);
    }

    void Window::SetFPS(int fps) { SetTargetFPS(fps); }
    bool Window::ShouldClose() { return WindowShouldClose(); }
    void Window::Close() { CloseWindow(); }
} // namespace myriad