#ifndef MYRIAD_CORE_MYRENTRYPOINT_H
#define MYRIAD_CORE_MYRENTRYPOINT_H

#include "core/MyrApplication.h"

extern Myriad::MyrApplication *Myriad::CreateApplication();
int main(int argc, char **argv)
{
  /*
  Myriad::Log::Init();
  MYR_CORE_INFO("Welcome to Myriad Engine");
  MYR_CORE_INFO("Info message");
  MYR_CORE_TRACE("TRACE message");
  MYR_CORE_WARN("WARN message");
  MYR_CORE_ERROR("Inited Myriad Log");

  MYR_INFO("Inited Client Log");
  int a = 7;
  MYR_TRACE("Test: a={0}", a);
  */
  auto app = Myriad::CreateApplication();
  app->Run();
  delete app;
  return 0;
}

#endif
