#ifndef MYRIAD_CORE_MYRENTRYPOINT_H
#define MYRIAD_CORE_MYRENTRYPOINT_H

#include "myriad.h" //for MyrApplication

extern Myriad::MyrApplication *Myriad::CreateApplication();

int main(int argc, char **argv)
{
  auto app = Myriad::CreateApplication();
  app->Run();
  delete app;
  return 0;
}
#endif
