#ifndef MYRIAD_IO_KEYBOARDINPUTPROVIDERRAYLIB_H
#define MYRIAD_IO_KEYBOARDINPUTPROVIDERRAYLIB_H

#include "core/core.h"
#include "io/KeyboardInputProvider.h"
#include "io/Log.h"

#include "raylib.h"

namespace Myriad
{
    class MYR_API KeyboardInputProviderRaylib : public KeyboardInputProvider
    {
      public:
        ~KeyboardInputProviderRaylib(){}
        bool Shutdown() override;
        bool Init() override;
        bool IsKeyPressed(keycode_t key) override;
        bool IsKeyReleased(keycode_t key) override;
        bool IsKeyDown(keycode_t key) override;
        bool IsKeyUp(keycode_t key) override;
    };

    bool KeyboardInputProviderRaylib::Shutdown()
    {
        MYR_CORE_INFO("Keyboard Input Provider [Raylib] shutting down");
        return true;
    }

    bool KeyboardInputProviderRaylib::Init()
    {
        MYR_CORE_INFO("Keyboard Input Provider [Raylib] init");
        return true;
    }

    bool KeyboardInputProviderRaylib::IsKeyPressed(keycode_t key)
    {
        return ::IsKeyPressed(key);
    }
    bool KeyboardInputProviderRaylib::IsKeyReleased(keycode_t key)
    {
        return ::IsKeyReleased(key);
    }
    bool KeyboardInputProviderRaylib::IsKeyDown(keycode_t key)
    {
        return ::IsKeyDown(key);
    }
    bool KeyboardInputProviderRaylib::IsKeyUp(keycode_t key)
    {
        return IsKeyUp(key);
    }
} // namespace Myriad

#endif
