// TODO: get rid of this, which is complicated
// because of types2d.h but should be fixable.
// #include "raylib.h"

#include <core/MyrEntryPoint.h>
#include <myriad.h>

#include "TestRenderer.h"

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
        Myriad::GameEntity *gentity = new Myriad::GameEntity();

        Myriad::Scene::Scene *pMyscene = new Myriad::Scene::Scene("MyScene");

        pMyscene->AddGameObject(*gobject);
        pMyscene->AddGameObject(*gentity);

        TestGameObject testObject1;
        Myriad::Transform testObjectTransform = testObject1.GetTransform();

        std::cout << "Transform type: " << Myriad::Transform::GetType().name()
                  << std::endl;

        // TODO: this didn't work
        testObjectTransform.SetPosition(400, 400, 0);
        Myriad::TransformData *tdata =
            (Myriad::TransformData *)testObjectTransform.Data();
        MYR_TRACE("Tdata address = {0:x}", (size_t)tdata);
        MYR_TRACE("Tdata = {0}, {1}", tdata->position.x, tdata->position.y);
        // TODO: had to resort to this
        tdata->position.x = 400;
        tdata->position.y = 400;

        std::list<Myriad::GameObject *> drawObjects;
        drawObjects.push_back(&testObject1);

        Myriad::RendererGroup rgroup;
        rgroup.add(testObject1.GetRenderer());

        while (!win->ShouldClose())
        {
            cam->Draw(rgroup);
            // Want to use these on windows? Need to build 'special'
            // See Readme.md
            // BeginDrawing();
            // ClearBackground({0, 0, 128, 255});
            // EndDrawing();
        }

        pMyscene->RemoveGameObject(*gobject);
        pMyscene->RemoveGameObject(*gentity);

        rgroup.remove(testObject1.GetRenderer());

        delete gobject;
        delete gentity;
        delete pMyscene;
    }
};

Myriad::MyrApplication *Myriad::CreateApplication() { return new Test(); }