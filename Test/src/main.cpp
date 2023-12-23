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

        Myriad::Window *w = new Myriad::Window();
        w->SetFPS(60);
        w->Init(800, 600, "Test Window");

        while (!w->ShouldClose())
        {
            // Something in here is causing that crash on windows...
            // And it's not clearbackground...
            BeginDrawing();
            //   ClearBackground({0, 0, 0, 0});
            EndDrawing();
        }
    }
};

Myriad::MyrApplication *Myriad::CreateApplication() { return new Test(); }