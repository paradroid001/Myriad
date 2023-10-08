#include "Dot.h"
#include <list>
#include <myriad.h>

class Sample : public Myriad::MyrApplication
{
  public:
    Sample();
    ~Sample();
    void Run(); // override;

    void Update();
    void Draw();

    std::list<Myriad::GameObject *> *pObjects;
    Myriad::Camera *camera;
};