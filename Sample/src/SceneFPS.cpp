#ifndef __SAMPLEFPSSCENE_H_
#define __SAMPLEFPSSCENE_H_
// #include "Scene/Scene.h"
// #include "SceneGameOver.cpp"
// #include "core/GameObject.h"
#include "myriad.h"

#include "raylib.h"
#include "rcamera.h"

#define MAX_COLUMNS 20

class SampleFPSScene : public Myriad::Scene::Scene
{
  public:
    SampleFPSScene(const char *name) : Myriad::Scene::Scene(name){};
    void LoadScene() override;
    void RunScene() override;
    void DrawScene() override;

  protected:
    Myriad::MyrColour bgColour;
    std::list<Myriad::GameObject *> *pObjects;
    flecs::world world;
    flecs::system updateSystem;
    flecs::system renderSystem;
    Camera rcamera; // raylib camera
    bool inited = false;

    // Generates some random columns
    float heights[MAX_COLUMNS] = {0};
    Vector3 positions[MAX_COLUMNS] = {0};
    Color colors[MAX_COLUMNS] = {0};
};

void SampleFPSScene::LoadScene()
{
    pObjects = new std::list<Myriad::GameObject *>();
    bgColour = {.r = 200, .g = 100, .b = 200, .a = 255};
    std::cout << "Creating FPS Scene" << std::endl;
    if (inited == false)
    {
        rcamera = {0};
        rcamera.position = (Vector3){0.0f, 2.0f, 4.0f};
        rcamera.target = (Vector3){0.0f, 2.0f, 0.0f};
        rcamera.up = (Vector3){0.0f, 1.0f, 0.0f};
        rcamera.fovy = 60.0f;
        rcamera.projection = CAMERA_PERSPECTIVE;

        for (int i = 0; i < MAX_COLUMNS; i++)
        {
            heights[i] = (float)GetRandomValue(1, 12);
            positions[i] =
                (Vector3){(float)GetRandomValue(-15, 15), heights[i] / 2.0f,
                          (float)GetRandomValue(-15, 15)};
            colors[i] = (Color){(unsigned char)GetRandomValue(20, 255),
                                (unsigned char)GetRandomValue(10, 55), 30,
                                (unsigned char)255};
        }
        DisableCursor(); // Limit cursor to relative movement inside the window

        // init the systems in here.
    }
    inited = true;
}

void SampleFPSScene::RunScene()
{
    // Update camera computes movement internally depending on the camera
    // mode Some default standard keyboard/mouse inputs are hardcoded to
    // simplify use For advance camera controls, it's reecommended to
    // compute camera movement manually
    UpdateCamera(&rcamera, 0); // cameraMode); // Update camera
    if (updateSystem != NULL)
    {
        updateSystem.run();
    }
}

void SampleFPSScene::DrawScene()
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

    // Draw some cubes around
    for (int i = 0; i < MAX_COLUMNS; i++)
    {
        DrawCube(positions[i], 2.0f, heights[i], 2.0f, colors[i]);
        DrawCubeWires(positions[i], 2.0f, heights[i], 2.0f, MAROON);
    }

    // Draw player cube
    // if (cameraMode == CAMERA_THIRD_PERSON)
    //{
    DrawCube(rcamera.target, 0.5f, 0.5f, 0.5f, PURPLE);
    DrawCubeWires(rcamera.target, 0.5f, 0.5f, 0.5f, DARKPURPLE);
    //}

    EndMode3D();

    // Draw info boxes
    DrawRectangle(5, 5, 330, 100, Fade(SKYBLUE, 0.5f));
    DrawRectangleLines(5, 5, 330, 100, BLUE);

    EndDrawing();
}
#endif