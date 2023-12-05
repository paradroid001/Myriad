#ifndef __MAIN_H_
#define __MAIN_H_

#include <list>

#include "TestEvent.h"
#include "flecs.h"
#include <myriad.h>

class Sample : public Myriad::MyrApplication
{
  public:
    Sample();
    ~Sample();
    void Run(); // override;

    void Update();
    void Draw();

    void EventHandler(TestEvent *e);

  protected:
    std::list<Myriad::GameObject *> *pObjects;
    Myriad::Camera *camera;
    int screenHeight;
    int screenWidth;
    int framesCounter;

    Myriad::Scene::Scene *menuScene;
    Myriad::Scene::Scene *dotsScene;
    Myriad::Scene::Scene *fpsScene;
    Myriad::Scene::Scene *fpsecsScene;
    Myriad::Scene::Scene *gameOverScene;
    Myriad::Scene::Scene *currentScene;
};

#endif