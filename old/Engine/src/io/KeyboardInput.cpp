#include "io/KeyboardInput.h"
#ifdef MYRIAD_INTERNAL
    #include "core/MyriadConfig.h"
#else
// you can forward declare functions and classes here.
#endif

// Match the keyboard provider to the renderer.
#if MYRIAD_RENDERER == RAYLIB
    #include "io/KeyboardInputProviderRaylib.h"
// Other render providers can go here.
#endif
namespace Myriad
{
    KeyboardInput::KeyboardInput() {}
    KeyboardInput::~KeyboardInput() {}
    bool KeyboardInput::Init()
    {
#if MYRIAD_RENDERER == RAYLIB
        p_keyboard_input_provider_ = static_cast<KeyboardInputProvider *>(
            new KeyboardInputProviderRaylib());
#endif
        return true;
    }
    bool KeyboardInput::Shutdown()
    {
        delete p_keyboard_input_provider_;
        return true;
    }
    bool KeyboardInput::IsKeyPressed(keycode_t keycode)
    {
        return p_keyboard_input_provider_->IsKeyPressed(keycode);
    }
    bool KeyboardInput::IsKeyReleased(keycode_t keycode)
    {
        return p_keyboard_input_provider_->IsKeyReleased(keycode);
    }
    bool KeyboardInput::IsKeyUp(keycode_t keycode)
    {
        return p_keyboard_input_provider_->IsKeyUp(keycode);
    }
    bool KeyboardInput::IsKeyDown(keycode_t keycode)
    {
        return p_keyboard_input_provider_->IsKeyDown(keycode);
    }

} // namespace Myriad
