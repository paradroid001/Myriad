#ifndef __SAMPLEMENUSCENE_H_
#define __SAMPLEMENUSCENE_H_

#include "myriad.h"
#include "raylib.h"

class SampleMenuScene : public Myriad::Scene::Scene
{
  public:
    SampleMenuScene(const char *name) : Myriad::Scene::Scene(name){};
    void LoadScene() override;
    void RunScene() override;
    void DrawScene() override;

  protected:
    Myriad::MyrColour bgColour;
    int framesCounter;
};

void SampleMenuScene::LoadScene()
{
    bgColour = {0, 0, 0, 255};
    framesCounter = 0;
}

void SampleMenuScene::RunScene()
{
    MYR_TRACE("Running menu scene, adding to framecounter");
    framesCounter += 1;
}

void SampleMenuScene::DrawScene()
{
    MYR_TRACE("Drawing menu scene");

    int screenWidth = 800;
    int screenHeight = 600;

    BeginDrawing();
    Color c;
    c.r = bgColour.r;
    c.g = bgColour.g;
    c.b = bgColour.b;
    c.a = bgColour.a;
    ClearBackground(c);

    DrawRectangle(0, 0, screenWidth, screenHeight, BLUE);
    DrawText("MENU SCREEN", 20, 20, 40, DARKBLUE);
    DrawText("Wait to enter DOTS screen", 120, 220, 20, DARKBLUE);

    EndDrawing();
    MYR_TRACE("Finished Drawing menu scene");
}

#endif