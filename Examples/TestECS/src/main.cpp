#include "myriad.h"
#include "MyrEntrypoint.h"
#include "TestECS.h"

Myriad::MyrApplication *Myriad::CreateApplication()
{
  return new TestECS();
}
