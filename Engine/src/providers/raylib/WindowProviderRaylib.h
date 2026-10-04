#ifndef _MYRIAD_ENGINE_WINDOW_PROVIDER_RAYLIB_H_
#define _MYRIAD_ENGINE_WINDOW_PROVIDER_RAYLIB_H_

#include "myriad_types.h"
#include "raylib.h"

namespace Myriad
{
    class WindowProviderRaylib : public IWindowProvider
    {
      protected:
        WindowState_t state_ = WindowState_t::CLOSED;

      public:
        WindowProviderRaylib() = default;
        virtual ~WindowProviderRaylib() = default;

        virtual WindowState_t GetState() override;
        virtual bool Open(const WindowConfig &config) override;
        virtual void Close() override;
        virtual bool Init() override;
        virtual void Shutdown() override;
    };
} // namespace Myriad
#endif // _MYRIAD_ENGINE_WINDOW_PROVIDER_RAYLIB_H_
