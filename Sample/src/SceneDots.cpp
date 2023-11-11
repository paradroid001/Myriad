#ifndef __SAMPLEDOTSSCENE_H_
#define __SAMPLEDOTSSCENE_H_

#include "Dot.h"
// #include "Entities/EntityManager.h"
// #include "flecs.h"
#include "core/Component.h"
#include "myriad.h"
#include "raylib.h"

struct VelocityData : Myriad::ComponentData
{
    float x;
    float y;
};

class DotVelocity : public Myriad::Component
{
  public:
    DotVelocity() : Myriad::Component() { SetVelocity(0, 0); };
    // DotVelocity(float _x, float _y){};
    inline void SetVelocity(float _x, float _y)
    {
        this->_internalData.x = _x;
        this->_internalData.y = _y;
    }

  protected:
    VelocityData _internalData;
};

class SampleDotsScene : public Myriad::Scene::Scene
{
  public:
    SampleDotsScene(const char *name) : Myriad::Scene::Scene(name){};
    void LoadScene() override;
    void RunScene() override;
    void DrawScene() override;

  protected:
    Myriad::MyrColour bgColour;
    std::list<Myriad::GameObject *> *pObjects;
    flecs::world world;
    flecs::system sys;
    flecs::system renderdots;
    bool inited = false;
};

void SampleDotsScene::LoadScene()
{
    pObjects = new std::list<Myriad::GameObject *>();
    bgColour = {0, 0, 0, 255};

    for (int i = 0; i < 1000; i++)
    {
        Dot *dot = new Dot();
        Myriad::Transform *const t = dot->GetTransform();
        t->SetPosition(rand() % 800, rand() % 600, 0);

        std::cout << dot->Entity().type().str() << std::endl;

        auto dot_e = dot->Entity();

        // set the position of each dot.
        Myriad::ComponentData *p_cd = t->Data();
        Myriad::TransformData *p_td =
            static_cast<Myriad::TransformData *>(p_cd);
        dot_e.set<Myriad::TransformData>(*(p_td));

        DotVelocity *v = new DotVelocity();
        // v->SetVelocity(-1, -1);
        //  VelocityData *vd = new VelocityData();
        dot->AddComponent<VelocityData>(v, v->Data());
        // v->SetVelocity((rand() % 2) - 1, (rand() % 2) - 1);
        // v->SetVelocity(1, 2);
        // dot_e.set<VelocityData>(*(static_cast<VelocityData *>(v->Data())));
        VelocityData vdata;
        vdata.x = (rand() % 4) - 2;
        vdata.y = (rand() % 4) - 2;
        dot_e.set<VelocityData>(vdata);
        // pObjects->push_front(dot);
    }

    if (inited == false)
    {
        // declare system
        sys = Myriad::Entities::EntityManager::Instance()
                  ->World()
                  .system<Myriad::TransformData, VelocityData>("Move")
                  .each(
                      [](Myriad::TransformData &t, VelocityData &v)
                      {
                          // std::cout << " vel is " << v.x << ", " << v.y
                          //<< std::endl;
                          if ((t.position.x + v.x > 800) ||
                              (t.position.x + v.x < 0))
                              v.x = -v.x;
                          if ((t.position.y + v.y > 600) ||
                              (t.position.y + v.y < 0))
                              v.y = -v.y;

                          // Each is invoked for each entity
                          t.position.x += v.x;
                          t.position.y += v.y;

                          // std::cout << "t.position.x " << t.position.x <<
                          // std::endl;
                      });

        renderdots =
            Myriad::Entities::EntityManager::Instance()
                ->World()
                .system<Myriad::TransformData>("Render")
                .each(
                    [](const Myriad::TransformData &t) -> void
                    {
                        // DrawCircleShape(p.x, p.y, 2, {100, 200, 0,
                        // 255});
                        DrawCircle(t.position.x, t.position.y, 2, GREEN);
                        // std::cout << "Draw" << std::endl;
                    });
    }
    inited = true;
}

void SampleDotsScene::RunScene()
{
    // MYR_TRACE("Running Sampledots scene");
    if (sys != NULL)
    {
        sys.run();
    }
}

void SampleDotsScene::DrawScene()
{
    int screenWidth = 800;
    int screenHeight = 600;

    BeginDrawing();
    Color c;
    c.r = bgColour.r;
    c.g = bgColour.g;
    c.b = bgColour.b;
    c.a = bgColour.a;
    ClearBackground(c);

    // DrawRectangle(0, 0, screenWidth, screenHeight, GREEN);
    // DrawText("DOTS SCREEN", 20, 20, 40, DARKGREEN);
    // DrawText("PRESS ENTER or TAP to JUMP to MENU SCREEN", 120, 220, 20,
    //          DARKGREEN);

    // MYR_TRACE("Drawing Sampledots scene");
    renderdots.run();

    EndDrawing();
}

#endif