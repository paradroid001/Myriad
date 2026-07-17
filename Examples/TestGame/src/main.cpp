#include "myriad.h"
#include "MyrEntrypoint.h"
#include "TestGame.h"

Myriad::MyrApplication *Myriad::CreateApplication()
{
  return new TestGame();
}
