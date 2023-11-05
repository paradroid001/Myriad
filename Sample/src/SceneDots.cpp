#ifndef __SAMPLEDOTSSCENE_H_
#define __SAMPLEDOTSSCENE_H_

#include "core/core.h"
#include "myriad.h"
#include "raylib.h"

class SampleDotsScene : public Myriad::Scene::Scene
{
  public:
    SampleDotsScene(const char* name) : Myriad::Scene::Scene(name){};
    void LoadScene() override;
    void RunScene() override;
    void DrawScene() override;

  protected:
    Myriad::MyrColour bgColour;
};

void SampleDotsScene::LoadScene() { bgColour = {0, 0, 0, 255}; }

void SampleDotsScene::RunScene() {
  MYR_TRACE("Running Sampledots scene");
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

    DrawRectangle(0, 0, screenWidth, screenHeight, GREEN);
    DrawText("DOTS SCREEN", 20, 20, 40, DARKGREEN);
    DrawText("PRESS ENTER or TAP to JUMP to MENU SCREEN", 120, 220, 20,
             DARKGREEN);

    EndDrawing();
}

#endif