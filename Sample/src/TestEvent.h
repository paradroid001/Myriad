#ifndef __TEST_EVENT_H_
#define __TEST_EVENT_H_

#include "myriad.h"

class TestEvent : public Myriad::Events::Event<TestEvent>
{
  public:
    int somedata1;
    int somedata2;
};

#endif