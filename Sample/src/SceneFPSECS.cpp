#ifndef __SAMPLEFPSECSSCENE_H_
#define __SAMPLEFPSECSSCENE_H_

#include "Entities/EntityManager.h"
#include "Scene/Scene.h"
#include "core/Component.h"
#include "core/GameObject.h"
#include "core/Log.h"
#include "core/Renderer.h"
#include "core/core.h"
#include "myriad.h"

#include "raylib.h"
#include "rcamera.h"

#define MAX_COLS 20

struct BoxRendererData : public Myriad::RendererData
{
    Myriad::Vector3 position;
    Myriad::MyrColour colour;

    BoxRendererData()
    {
        MYR_TRACE("Initing BoxRendererData");
        position = {0, 0, 0};
        colour = {255, 0, 128, 255};
    }

    void Print()
    {
        MYR_TRACE("BoxRendererData: [{0:x}] {1},{2},{3}", (long long)pcomponent,
                  position.x, position.y, position.z);
    }
};

class BoxRenderer : public Myriad::Renderer
{
  public:
    BoxRenderer() : Myriad::Renderer()
    {
        MYR_TRACE("Initing BoxRenderer");
        MYR_TRACE("Setting componentdata object to a BoxRendererData");
        SetComponentData(&_boxRendererData);
        // TODO could this just be a constructor on BoxRendererData
        // or does that not work?
        MYR_TRACE("Setting some default values for box render data");
        SetRendererData({0, 0, 0}, {255, 0, 128, 255});
        MYR_TRACE("Finished Initing BoxRenderer");
    };
    inline void SetRendererData(Myriad::Vector3 v3, Myriad::MyrColour col)
    {
        _boxRendererData.position = v3;
        _boxRendererData.colour = col;
    };
    void Draw(Myriad::TransformData t)
    {
        // MYR_TRACE("Draw a box");
        //((BoxRendererData *)(&bxrd))->Print();

        // This code doesn't work, _boxRendererData isn't the right data.
        /*
        Vector3 pos = {_boxRendererData.position.x, _boxRendererData.position.y,
                       _boxRendererData.position.z};
        DrawCube(pos, 2.0f, pos.y * 2.0f, 2.0f,
                 {_boxRendererData.colour.r, _boxRendererData.colour.g,
                  _boxRendererData.colour.b, _boxRendererData.colour.a});
        DrawCubeWires(pos, 2.0f, pos.y * 2.0f, 2.0f, MAROON);
        */
        flecs::entity boxRendererEntity = _pgameObject->Entity();
        const BoxRendererData *bxrd = boxRendererEntity.get<BoxRendererData>();
        Vector3 pos = {bxrd->position.x, bxrd->position.y, bxrd->position.z};
        DrawCube(
            pos, 2.0f, pos.y * 2.0f, 2.0f,
            {bxrd->colour.r, bxrd->colour.g, bxrd->colour.b, bxrd->colour.a});
        DrawCubeWires(pos, 2.0f, pos.y * 2.0f, 2.0f, MAROON);
    };

  protected:
    BoxRendererData _boxRendererData;
};

class Column : public Myriad::GameObject
{
  public:
    Column() : Myriad::GameObject()
    {
        // Give it a renderer component
        // BoxRenderer *br = new BoxRenderer();
        // AddComponent<BoxRendererData>(br);
    }
    ~Column() {}
    void Draw()
    {
        ///
        MYR_TRACE("This is the column draw function");
        ///
    };
    void Update()
    {
        ///
        //??
        ///
    }
};

class SampleFPSECSScene : public Myriad::Scene::Scene
{
  public:
    SampleFPSECSScene(const char *name) : Myriad::Scene::Scene(name){};
    void LoadScene() override;
    void RunScene() override;
    void DrawScene() override;

  protected:
    Myriad::MyrColour bgColour;
    std::list<Myriad::GameObject *> *pObjects;
    // flecs::world world;
    flecs::system updateSystem;
    flecs::system renderSystem;
    Camera rcamera; // raylib camera
    bool inited = false;
};

void SampleFPSECSScene::LoadScene()
{
    pObjects = new std::list<Myriad::GameObject *>();
    bgColour = {.r = 0, .g = 0, .b = 0, .a = 0};
    std::cout << "Creating FPS ECS Scene" << std::endl;
    if (inited == false)
    {
        rcamera = {0};
        rcamera.position = (Vector3){0.0f, 2.0f, 4.0f};
        rcamera.target = (Vector3){0.0f, 2.0f, 0.0f};
        rcamera.up = (Vector3){0.0f, 1.0f, 0.0f};
        rcamera.fovy = 60.0f;
        rcamera.projection = CAMERA_PERSPECTIVE;

        for (int i = 0; i < MAX_COLS; i++)
        {
            float height = (float)GetRandomValue(1, 12);
            float x = (float)GetRandomValue(-15, 15);
            float z = (float)GetRandomValue(-15, 15);
            Myriad::MyrColour colour = {(unsigned char)GetRandomValue(20, 255),
                                        (unsigned char)GetRandomValue(10, 55),
                                        30, (unsigned char)255};

            Column *column = new Column();
            auto column_e = column->Entity();
            BoxRenderer *br = new BoxRenderer();
            MYR_TRACE("Adding boxrenderer to column");
            column->AddComponent<BoxRendererData>(br);
            // column_e.get<BoxRendererData>().pcomponent = br;
            MYR_TRACE("Finished adding boxrenderer to column");
            MYR_TRACE("Setting box render data on entity");
            Myriad::ComponentData *p_cd = br->Data();
            BoxRendererData *p_brd = static_cast<BoxRendererData *>(p_cd);
            BoxRendererData brd;
            // br->SetRendererData({x, height / 2, z}, colour);
            p_brd = &brd;
            p_brd->position.x = x;
            p_brd->position.y = height; // / 2.0f;
            p_brd->position.z = z;
            p_brd->colour = colour;
            p_brd->pcomponent = br;
            column_e.set<BoxRendererData>(*(p_brd));
            brd.Print();
            MYR_TRACE("End Set box render data on entity");
            //   column->GetComponent<BoxRenderer>().SetComponentData();
            pObjects->push_front(column);
        }
        DisableCursor(); // Limit cursor to relative movement inside the window

        // init the systems in here.
        renderSystem =
            Myriad::Entities::EntityManager::Instance()
                ->World()
                .system<Myriad::TransformData, BoxRendererData>("BoxRender")
                .each(
                    [](const Myriad::TransformData &t,
                       const BoxRendererData &bxrd) -> void
                    {
                        // MYR_TRACE("It's draw time");
                        if (bxrd.pcomponent != NULL &&
                            (long long)(bxrd.pcomponent) != 0x6021)
                        {
                            MYR_TRACE("The component is: {0:x}",
                                      (long long)bxrd.pcomponent);

                            // BoxRenderer Component Draw
                            ((BoxRenderer *)bxrd.pcomponent)->Draw(t);
                        }

                        /*
                        //((BoxRendererData *)(&bxrd))->Print();
                        Vector3 pos = {bxrd.position.x, bxrd.position.y,
                                       bxrd.position.z};
                        DrawCube(pos, 2.0f, 7 * 2.0f, 2.0f,
                                 {bxrd.colour.r, bxrd.colour.g, bxrd.colour.b,
                                  bxrd.colour.a});
                        DrawCubeWires(pos, 2.0f, 7 * 2.0f, 2.0f, MAROON);
                        */
                    });
    }
    inited = true;
}

void SampleFPSECSScene::RunScene()
{
    UpdateCamera(&rcamera, 0); // camera mode = 0
}
void SampleFPSECSScene::DrawScene()
{
    int screenWidth = 800;
    int sceeenHeight = 600;
    BeginDrawing();
    Color c;
    c.r = bgColour.r;
    c.g = bgColour.g;
    c.b = bgColour.b;
    c.a = bgColour.a;
    ClearBackground(c);

    BeginMode3D(rcamera);
    DrawPlane((Vector3){0.0f, 0.0f, 0.0f}, (Vector2){32.0f, 32.0f},
              LIGHTGRAY); // Draw ground
    DrawCube((Vector3){-16.0f, 2.5f, 0.0f}, 1.0f, 5.0f, 32.0f,
             BLUE); // Draw a blue wall
    DrawCube((Vector3){16.0f, 2.5f, 0.0f}, 1.0f, 5.0f, 32.0f,
             LIME); // Draw a green wall
    DrawCube((Vector3){0.0f, 2.5f, 16.0f}, 32.0f, 5.0f, 1.0f,
             GOLD); // Draw a yellow wall

    // MYR_TRACE("Running rendersystem");
    renderSystem.run();
    EndMode3D();

    // Draw info boxes
    DrawRectangle(5, 5, 330, 100, Fade(SKYBLUE, 0.5f));
    DrawRectangleLines(5, 5, 330, 100, BLUE);
    EndDrawing();
}

#endif