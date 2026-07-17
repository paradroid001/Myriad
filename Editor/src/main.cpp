#include "editor_app.h"

#include "MyrEntrypoint.h"

Myriad::MyrApplication *Myriad::CreateApplication()
{
  return new MyriadEditor();
}
