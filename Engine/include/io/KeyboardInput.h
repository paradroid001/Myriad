#ifndef MYRIAD_IO_KEYBOARDINPUT_H
#define MYRIAD_IO_KEYBOARDINPUT_H

#include "core/core.h"
#include "io/KeyboardInputProvider.h"

namespace Myriad
{
  class MYR_API KeyboardInput : public KeyboardInputProvider
  {
  private:
    KeyboardInputProvider *p_keyboard_input_provider_;

  public:
    KeyboardInput();
    ~KeyboardInput();
    bool Init() override;
    bool Shutdown() override;
    bool IsKeyPressed(KeyCode_t key) override;
    bool IsKeyReleased(KeyCode_t key) override;
    bool IsKeyDown(KeyCode_t key) override;
    bool IsKeyUp(KeyCode_t key) override;
  };
} // namespace Myriad

#endif
