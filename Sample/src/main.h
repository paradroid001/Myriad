#include <myriad.h>

class Sample : public Myriad::MyrApplication
{
  public:
    Sample();
    ~Sample();
    void Run(); // override;
};