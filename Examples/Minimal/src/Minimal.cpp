#include "myriad.h"
#include "myriad_config.h"
#include "Minimal.h"

Minimal::Minimal()
{
  MYR_TRACE("Creating Minimal Application");
}

Minimal::~Minimal()
{
  MYR_TRACE("Destroying Minimal Application");
}

void Minimal::Update(float delta_ms)
{
  // MYR_TRACE("Updating Minimal Application: delta_ms = %f", delta_ms);
}
void Minimal::Render(float delta_ms)
{
  // MYR_TRACE("Rendering Minimal Application: delta_ms = %f", delta_ms);
  renderer_->DrawText(Myriad::FONTHANDLE_INVALID, "Hello", {100, 100}, 20, {255, 0, 0, 255});
}
