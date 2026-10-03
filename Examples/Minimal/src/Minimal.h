#ifndef MINIMAL_H
#define MINIMAL_H

#include "myriad.h"

class Minimal : public Myriad::GameApplication
{
public:
  Minimal();
  virtual ~Minimal();

  void Update(float delta_ms) override;
  void Render(float delta_ms) override;
};

#endif // MINIMAL_H
