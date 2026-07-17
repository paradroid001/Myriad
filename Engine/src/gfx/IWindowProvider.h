#ifndef _MYRIAD_GFX_IWINDOWPROVIDER_H_
#define _MYRIAD_GFX_IWINDOWPROVIDER_H_

#include "myriad.h" //IProvider, WindowState_t

namespace Myriad
{
    class IWindowProvider : public IWindow, public IProvider
    {
      public:
        virtual ~IWindowProvider() = 0;

        // Interface Methods (IProvider)
        virtual bool Init() = 0;
        virtual bool Shutdown() = 0;

        // Interface Methods (IWindow)
        virtual bool Open(WindowConfig config, const char *title) = 0;
        virtual void SetFPS(int fps) = 0;
        virtual bool Close() = 0;

        virtual bool WasResized() = 0;
        virtual WindowState_t GetWindowState() = 0;
        virtual Vector2 GetDimensions() = 0;

        virtual bool WindowHasProp(WindowPropFlags_t prop_flag) = 0;
        virtual bool WindowSetProp(WindowPropFlags_t prop_flag, bool val) = 0;
        virtual WindowProps_t GetWindowProps() = 0;
    };
} // namespace Myriad

#endif
