#ifndef _MYRIAD_IRENDERERPROVIDER_H_
#define _MYRIAD_IRENDERERPROVIDER_H_

#include "myriad.h" //IProvider, Vector2, etc

namespace Myriad
{
    class Texture2DProvider; // fwd declare
    class IRendererProvider : public IProvider, public IRenderer
    {

      public:
        // IRendererProvider(Texture2DProvider *tex_provider) : IProvider() {}

        virtual ~IRendererProvider() = 0;
        // Interface Methods (IProvider)
        virtual bool Init() = 0;
        virtual bool Shutdown() = 0;

        // Interface Methods (IRenderer)
        virtual void ClearBackground(MyrColour colour) = 0;
        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;

        virtual void DrawRectangle(Vector2 pos, Vector2 size,
                                   MyrColour colour) = 0;
        virtual void DrawCircle(Vector2 pos, float radius,
                                MyrColour colour) = 0;
        virtual void DrawTexture(AssetID_t tex_asset, Vector2 pos,
                                 MyrColour colour) = 0;
        virtual void DrawTexture(AssetID_t tex_asset, Rect2D rect, Vector2 pos,
                                 MyrColour colour) = 0;
        virtual void DrawText(Asset *font, std::string text, Vector2 pos,
                              int size, MyrColour colour) = 0;
    };

    inline IRendererProvider::~IRendererProvider() =
        default; // define ~IRendererProvider
} // namespace Myriad

#endif
