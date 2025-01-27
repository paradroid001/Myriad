#ifndef MYRIAD_IO_KEYBOARDPROVIDER_H
#define MYRIAD_IO_KEYBOARDPROVIDER_H

#include "core/IProvider.h"
#include "core/core.h"

namespace Myriad
{
  typedef int KeyCode_t;

  class KeyboardInputProvider : public IProvider
  {
  public:
    virtual ~KeyboardInputProvider() {}
    virtual bool Shutdown() = 0;
    virtual bool Init() = 0;
    virtual bool IsKeyPressed(KeyCode_t key) = 0;
    virtual bool IsKeyReleased(KeyCode_t key) = 0;
    virtual bool IsKeyDown(KeyCode_t key) = 0;
    virtual bool IsKeyUp(KeyCode_t key) = 0;
  };
} // namespace Myriad

/*
// Input-related functions: keyboard
    bool IsKeyPressed(int key); // Check if a key has been pressed once
    bool IsKeyPressedRepeat(int key); Check if a key has been pressed again
(Only PLATFORM_DESKTOP) bool IsKeyDown(int key);     // Check if a key is being
pressed bool IsKeyReleased(int key); // Check if a key has bee released once
    bool IsKeyUp(int key); // Check if a key is NOT being pressed
    int GetKeyPressed(void); // Get key pressed (keycode),
                             //call it multiple times for keys queued,
                             //returns 0 when the queue is empty
    int GetCharPressed(void);  // Get char pressed (unicode),
                               // call it multiple times for chars queued,
                               // returns 0 when the queue is empty
    void SetExitKey(int key);  // Set a custom key to exit program
                               // (default is ESC)
*/
#endif
