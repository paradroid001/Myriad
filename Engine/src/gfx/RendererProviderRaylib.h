#ifndef _MYRIAD_RENDERPROVIDER_RAYLIB_H_
#define _MYRIAD_RENDERPROVIDER_RAYLIB_H_
#include <memory>

#include "myriad.h"

#include "gfx/IRenderProvider.h"

#include "asset/Texture2DProviderRaylib.h"
#include "raylib.h"

namespace Myriad
{
    class RendererProviderRaylib : public IRendererProvider
    {
      protected:
        std::shared_ptr<Texture2DProviderRaylib> texture2d_provider_;

      public:
        RendererProviderRaylib(
            std::shared_ptr<Texture2DProviderRaylib> tex_provider);
        virtual ~RendererProviderRaylib() override;

        // Interface Methods (IProvider)
        virtual bool Init() override;
        virtual bool Shutdown() override;

        // Interface Methods (IRendererProvider)
        virtual void ClearBackground(MyrColour colour) override;

        virtual void BeginFrame() override;
        virtual void EndFrame() override;

        virtual void DrawRectangle(Vector2 pos, Vector2 size,
                                   MyrColour colour) override;
        virtual void DrawCircle(Vector2 pos, float radius,
                                MyrColour colour) override;
        virtual void DrawTexture(AssetID_t tex_asset, Vector2 pos,
                                 MyrColour colour) override;
        virtual void DrawTexture(AssetID_t tex_asset, Rect2D rect, Vector2 pos,
                                 MyrColour colour) override;
        virtual void DrawText(Asset *font, std::string text, Vector2 pos,
                              int size, MyrColour colour) override;
    };
} // namespace Myriad
#endif
