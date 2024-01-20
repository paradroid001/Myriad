// TODO: get rid of this, which is complicated
// because of types2d.h but should be fixable.
// #include "raylib.h"

#include <core/MyrEntryPoint.h>
#include <myriad.h>

//#include "raylib.h"
#include "TestRenderer.h"

#include <cstdlib>

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

        //Myriad::Log::SetLogLevelError(Myriad::Log::GetCoreLogger());
        Myriad::Log::SetLogLevelError(Myriad::Log::GetClientLogger());
        

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

        int numObjects = 800;
        TestGameObject objectsArray[numObjects];
        for (int i = 0; i < numObjects; i++)
        {
            MYR_TRACE("Game Object {0}", i);
            TestGameObject *tgo = &(objectsArray[i]);
            //MYR_TRACE("Getting Transform");
            Myriad::Transform& t = tgo->GetTransform();
            //Myriad::Transform *p_t = tgo->GetTransformP();
            
            
            //transform.SetPosition((float)(rand() % 100), (float)(rand() % 100),
            //                  (float)(rand() % 100));
            Myriad::ComponentData *cdata1 = t.Data();
            //MYR_TRACE("Address of data1 = {0:x}", (size_t)cdata1);
            //Myriad::ComponentData *cdata2 = p_t->Data();
            //MYR_TRACE("Address of data2 = {0:x}", (size_t)cdata2);
            
            
            Myriad::TransformData *tdata =
                (Myriad::TransformData *)(cdata1);
            tdata->position.x = (float)(rand() % 400);
            tdata->position.y = (float)(rand() % 400);
            rgroup.Add(tgo->GetRenderer());
            ugroup.Add(tgo->GetUpdater());
            MYR_TRACE("Done with iteration {0}", i);
            //Notice that now we get component destructor
            //as this guy falls off the stack.
            //But we don't get a component created, because
            //i guess the copy constructor was used on
            //line 73.
            //I don't seem to be able to return a pointer to the
            //transform, it just crashes when i call ->Data() :(
        }

        while (!win->ShouldClose())
        {
            ugroup.Update(1);
            cam->Draw(rgroup);
            
            // Want to use these on windows? Need to build 'special'
            // See Readme.md
            //BeginDrawing();
            //ClearBackground({0, 0, 128, 255});
            //testObject1.Draw();
            //EndDrawing();
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