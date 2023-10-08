#include <cstdlib> //rand
#include <list>

#include "Dot.h"
// #include "Remotery.h"
#include "main.h"

#include <myriad.h>
// Entry Point
#include <core/MyrEntryPoint.h>

Sample::Sample() {}
Sample::~Sample() {}

void Sample::Run()
{
    Myriad::Window *w = new Myriad::Window();
    MYR_TRACE("Made window class");
    w->SetFPS(30);
    MYR_TRACE("Set FPS");
    w->Init(800, 600, "Hello");
    MYR_TRACE("Inited windows");

    camera = new Myriad::Camera();
    pObjects = new std::list<Myriad::GameObject *>();

    for (int i = 0; i < 600; i++)
    {
        Dot *dot = new Dot();
        Myriad::Transform *const t = dot->GetTransform();
        t->SetPosition(rand() % 800, rand() % 600, 0);

        pObjects->push_front(dot);
    }

    // Create the main instance of Remotery.
    // You need only do this once per program.
    // Remotery *rmt;
    // rmt_CreateGlobalInstance(&rmt);
    // rmt_BindOpenGL();

    while (!w->ShouldClose())
    {
        this->Update();
        // rmt_BeginOpenGLSample(UnscopedSample);
        this->Draw();
        // rmt_EndOpenGLSample();
    }
    w->Close();
    delete w;

    // rmt_UnbindOpenGL();
    // rmt_DestroyGlobalInstance(rmt);
    MYR_TRACE("Cleaned up and quit");
}

void Sample::Update()
{
    // rmt_BeginCPUSample(Update, RMTSF_Aggregate);
    std::list<Myriad::GameObject *>::iterator it;
    for (it = pObjects->begin(); it != pObjects->end(); ++it)
    {
        (*it)->Update();
    }
    // rmt_EndCPUSample();
}

void Sample::Draw()
{
    // rmt_BeginCPUSample(Draw, RMTSF_Aggregate);
    camera->Draw(pObjects);
    // rmt_EndCPUSample();
}

Myriad::MyrApplication *Myriad::CreateApplication()
{
    // implementation of CreateApplication
    return new Sample();
}