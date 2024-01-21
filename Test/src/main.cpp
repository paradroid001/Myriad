// TODO: get rid of this, which is complicated
// because of types2d.h but should be fixable.
// #include "raylib.h"

#include <core/MyrEntryPoint.h>
#include <myriad.h>

#include "TestRenderer.h"

#include <cstdlib>

class Test : public Myriad::MyrApplication
{
  public:
    Myriad::AudioManager &audioManager = Myriad::AudioManager::GetInstance();
    Myriad::InputManager &inputManager = Myriad::InputManager::GetInstance();

    Test()
    {
        // Init the application here
        MYR_INFO("Test starting");
        audioManager.SetChannelVolume("default", 0.4f);
        Myriad::Json Json;
        nlohmann::json data = Json.ReadJson("example.json");
        std::cout << data.dump(4) << std::endl;
    };

    ~Test() { MYR_INFO("Test destructing"); };

    void Run()
    {
        MYR_INFO("Test is running!");

        // Myriad::Log::SetLogLevelError(Myriad::Log::GetCoreLogger());
        // Myriad::Log::SetLogLevelError(Myriad::Log::GetClientLogger());

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
        Myriad::UpdaterGroup ugroup;
        rgroup.Add(testObject1.GetRenderer());

        int numObjects = 1;
        TestGameObject objectsArray[numObjects];
        for (int i = 0; i < numObjects; i++)
        {
            MYR_TRACE("Game Object {0}", i);
            TestGameObject *tgo = &(objectsArray[i]);
            // Because this is a reference and not a full
            // local object, it won't trigger a destructor.
            Myriad::Transform &t = tgo->GetTransform();

            // To set the position you could do this:
            t.SetPosition((float)(rand() % 400), (float)(rand() % 400),
                          (float)(rand() % 400));
            // Or this:
            /*
            Myriad::ComponentData *cdata1 = t.Data();
            Myriad::TransformData *tdata =
                (Myriad::TransformData *)(cdata1);
            tdata->position.x = (float)(rand() % 400);
            tdata->position.y = (float)(rand() % 400);
            */

            rgroup.Add(tgo->GetRenderer());
            ugroup.Add(tgo->GetUpdater());
            MYR_TRACE("Done with iteration {0}", i);
        }

        while (!win->ShouldClose())
        {
            ugroup.Update(1);
            cam->Draw(rgroup);

            if (inputManager.IsKeyDown(Myriad::Key::KEY_A))
            {
                MYR_TRACE("A was pressed");
            }

            // Want to use these on windows? Need to build 'special'
            // See Readme.md
            // BeginDrawing();
            // ClearBackground({0, 0, 128, 255});
            // testObject1.Draw();
            // EndDrawing();
        }

        pMyscene->RemoveGameObject(*gobject);
        pMyscene->RemoveGameObject(*gentity);

        rgroup.Remove(testObject1.GetRenderer());

        delete gobject;
        delete gentity;
        delete pMyscene;
    }
};

Myriad::MyrApplication *Myriad::CreateApplication() { return new Test(); }