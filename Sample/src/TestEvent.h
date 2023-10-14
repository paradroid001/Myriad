#ifndef __TEST_EVENT_H_
#define __TEST_EVENT_H_

#include "Events/Event.h"
#include "myriad.h"

class TestEvent : public Myriad::Events::Event<TestEvent>
{
  public:
    TestEvent() : Myriad::Events::Event<TestEvent>(){};
    ~TestEvent();
    int somedata1;
    int somedata2;
    void Call(){};
};

#endif