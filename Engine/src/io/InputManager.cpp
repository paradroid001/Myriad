#include "io/InputManager.h"
#include "core/MyriadConfig.h"

namespace Myriad
{
    bool InputManager::IsKeyUp(Key key)
    {
        #if MYRIAD_RENDERER == RAYLIB
            return ::IsKeyUp((int)key);
        #endif
        return false;
    }
    bool InputManager::IsKeyDown(Key key)
    {
        #if MYRIAD_RENDERER == RAYLIB
            return ::IsKeyDown((int)key);
        #endif
        return false;
    }
    
    //pump the input polling, send out events.
    void Pump()
    {

    }
}