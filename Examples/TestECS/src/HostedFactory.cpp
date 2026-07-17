#include "TestECS.h"

extern "C" Myriad::MyrGameApplication *Myriad_CreateHostedGame()
{
  return new TestECS();
}

extern "C" void Myriad_DestroyHostedGame(Myriad::MyrGameApplication *application)
{
  delete application;
}
