#ifndef _MYRIAD_PROVIDERS_RAYLIB_RENDERPROVIDERRAYLIB_H_
#define _MYRIAD_PROVIDERS_RAYLIB_RENDERPROVIDERRAYLIB_H_

#include "myriad_types.h"
#include "raylib.h"

namespace Myriad
{
    class RenderProviderRaylib : public IRenderProvider
    {
      public:
        RenderProviderRaylib() = default;
        virtual ~RenderProviderRaylib() = default;

        virtual bool Init() override;
        virtual void Shutdown() override;

        virtual void BeginFrame() override;
        virtual void EndFrame() override;

        virtual void DrawText(AssetID_t font_id, const std::string &text,
                              Vector2 pos, int size, Colour colour) override;
    };
} // namespace Myriad

#endif // _MYRIAD_PROVIDERS_RAYLIB_RENDERPROVIDERRAYLIB_H_
