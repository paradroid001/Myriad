#include "raylib.h"
#include <core/MyrEntryPoint.h>
#include <myriad.h>

class Test : public Myriad::MyrApplication
{
  public:
    Test()
    {
        // Init the application here
        MYR_INFO("Test starting");
    };
    ~Test() { MYR_INFO("Test destructing"); };
    void Run()
    {
        MYR_INFO("Test is running!");

        Myriad::Window *win = new Myriad::Window();
        win->SetFPS(60);
        win->Init(800, 600, "Test Window");
        Myriad::Camera *cam = new Myriad::Camera();
        cam->SetBackgroundColour({0, 0, 0, 0});

        Myriad::GameObject *gobject = new Myriad::GameObject();

        while (!win->ShouldClose())
        {
            cam->Draw();
            // Want to use these on windows? Need to build 'special'
            // See Readme.md
            // BeginDrawing();
            // ClearBackground({0, 0, 128, 255});
            // EndDrawing();
        }

        delete gobject;
    }
};

Myriad::MyrApplication *Myriad::CreateApplication() { return new Test(); }