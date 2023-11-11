#ifndef __SAMPLEGAMEOVERSCENE_H_
#define __SAMPLEGAMEOVERSCENE_H_

#include "myriad.h"
#include "raylib.h"

class SampleGameOverScene : public Myriad::Scene::Scene
{
    public:
        SampleGameOverScene(const char* name) : Myriad::Scene::Scene(name){};
        void LoadScene() override;
        void RunScene() override;
        void DrawScene() override;
    protected:
        Myriad::MyrColour bgColour;
};

void SampleGameOverScene::LoadScene()
{
    bgColour = {0, 0, 0, 255};
}

void SampleGameOverScene::RunScene()
{
    //MYR_TRACE("Running game over scene");
}

void SampleGameOverScene::DrawScene()
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
    DrawText("GAME OVER SCREEN", 20, 20, 40, DARKGREEN);
    DrawText("Press SPACE to Restart", 120, 220, 20, DARKGREEN);

    EndDrawing();
}


#endif