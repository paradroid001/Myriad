#include "main.h"
#include "core/GameObject.h"
#include "core/Log.h"
#include <list>
#include <myriad.h>

#include "Dot.h"

Sample::Sample() {}
Sample::~Sample() {}

void Sample::Run()
{
    myriad::Window *w = new myriad::Window();
    MYR_TRACE("Made window class");
    w->SetFPS(30);
    MYR_TRACE("Set FPS");
    w->Init(800, 600, "Hello");
    MYR_TRACE("Inited windows");
    myriad::Camera *c = new myriad::Camera();

    std::list<myriad::GameObject> *pobjects;
    pobjects = new std::list<myriad::GameObject>();
    Dot *g = new Dot();

    pobjects->push_front(*g);

    while (!w->ShouldClose())
    {
        g->Update();
        c->Draw(pobjects);
    }
    w->Close();
    delete w;
}

Myriad::MyrApplication *Myriad::CreateApplication()
{
    // implementation of CreateApplication
    return new Sample();
}