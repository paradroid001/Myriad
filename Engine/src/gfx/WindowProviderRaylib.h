#ifndef _MYRIAD_GFX_WINDOWPROIVIDER_RAYLIB_H_
#define _MYRIAD_GFX_WINDOWPROIVIDER_RAYLIB_H_

#include "gfx/IWindowProvider.h"
#include "myriad.h"
#include "raylib.h"
namespace Myriad
{
    class WindowProviderRaylib : public IWindowProvider
    {
      protected:
        WindowState_t window_state_;
        WindowProps_t window_props_;

      public:
        WindowProviderRaylib();
        virtual ~WindowProviderRaylib() override;

        // Interface Methods (IProvider)
        virtual bool Init() override final;
        virtual bool Shutdown() override final;

        // Interface Methods (IWindow)
        virtual bool Open(WindowConfig config,
                          const char *title) override final;
        virtual void SetFPS(int fps) override final;
        virtual bool Close() override final;

        virtual WindowState_t GetWindowState() override final;
        virtual bool WasResized() override final;
        virtual Vector2 GetDimensions() override final;

        virtual bool WindowHasProp(WindowPropFlags_t prop_flag) override final;
        virtual bool WindowSetProp(WindowPropFlags_t prop_flag,
                                   bool val) override final;
        virtual WindowProps_t GetWindowProps() override final;

        virtual bool Minimise() override final;
        virtual bool Restore() override final;
        virtual bool Maximise() override final;
        virtual bool ToggleFullscreen() override final;
        virtual bool ToggleBorderlessWindowed() override final;
    };
} // namespace Myriad
#endif
