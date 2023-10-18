#ifndef __MAIN_H_
#define __MAIN_H_

#include <list>

#include "TestEvent.h"
#include "flecs.h"
#include <myriad.h>

struct Position
{
    float x;
    float y;
};
struct Velocity
{
    float x;
    float y;
};

class Sample : public Myriad::MyrApplication
{
  public:
    Sample();
    ~Sample();
    void Run(); // override;

    void Update();
    void Draw();

    void EventHandler(TestEvent *e);

    std::list<Myriad::GameObject *> *pObjects;
    Myriad::Camera *camera;
};

#endif