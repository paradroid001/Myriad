#include <iostream>
#include <thread>
#include <vector>
#include "Allocator.h"
#include "Handle.h"
#include "Tasks.h"

void myfunc(Handle handle)
{
  std::cout << "Myfunc did a thing" << std::endl;
}

void task1(std::string msg)
{
  std::cout << "task1 says: " << msg << std::endl;
}

int main(int argc, char **argv)
{
  std::thread t1(task1, "Hello");

  Allocator *allocator = new Allocator();
  Handle h = allocator->alloc();
  std::cout << "Calling myfunc" << std::endl;
  myfunc(h);
  std::cout << "Finished calling myfunc" << std::endl;

  // Make some tasks:
  Reporter *reporter_1 = new Reporter();
  Reporter *reporter_2 = new Reporter();
  Reporter *reporter_3 = new Reporter();
  Incrementer *incrementer_1 = new Incrementer();
  Incrementer *incrementer_2 = new Incrementer();
  Incrementer *incrementer_3 = new Incrementer();

  std::vector<std::thread> threads(6);
  int count = 0;
  while (count < 100)
  {
    threads[0] = std::thread(&Incrementer::Iterate, incrementer_1);
    threads[2] = std::thread(&Incrementer::Iterate, incrementer_2);
    threads[4] = std::thread(&Incrementer::Iterate, incrementer_3);
    threads[1] = std::thread(&Reporter::Report, reporter_1, *incrementer_1);
    threads[3] = std::thread(&Reporter::Report, reporter_2, *incrementer_2);
    threads[5] = std::thread(&Reporter::Report, reporter_3, *incrementer_3);
    for (auto &th : threads)
    {
      th.join();
    }
    count = incrementer_1->Get() + incrementer_2->Get() + incrementer_3->Get();
  }

  std::cout << "Program ending, joining threads" << std::endl;
  t1.join();
  return 0;
}
