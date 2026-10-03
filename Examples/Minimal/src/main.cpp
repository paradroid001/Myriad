#include "myriad.h"
#include "myriad_entrypoint.h"
#include "Minimal.h"

Myriad::Application *Myriad::CreateApplication()
{
  return new Minimal();
}
