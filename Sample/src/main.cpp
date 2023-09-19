#include <myriad.h>

class Sample : public Myriad::MyrApplication
{
  public:
    Sample() {}
    ~Sample() {}
};

Myriad::MyrApplication *Myriad::CreateApplication()
{
    // implementation of CreateApplication
    return new Sample();
}