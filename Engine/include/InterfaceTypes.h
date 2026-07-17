#ifndef _MYRIAD_INTERFACETYPES_H_
#define _MYRIAD_INTERFACETYPES_H_

#include "CoreTypes.h"    //Colour, WindowConfig, Vector2, Rect2D
#include "EngineConfig.h" //MYR_API

namespace Myriad
{

    class IProvider
    {
      public:
        virtual bool Init() = 0;
        virtual bool Shutdown() = 0;
    };

    class Asset; // fwd declare

    class MYR_API IRenderer
    {
      public:
        virtual ~IRenderer() = 0;
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

    // An interface for windows
    class MYR_API IWindow
    {
      public:
        virtual ~IWindow() = 0;
        virtual bool Open(WindowConfig config, const char *title) = 0;
        virtual void SetFPS(int fps) = 0;
        virtual bool Close() = 0;

        virtual WindowState_t GetWindowState() = 0;
        virtual bool WasResized() = 0;
        virtual Vector2 GetDimensions() = 0;

        // Ask about single property flag
        virtual bool WindowHasProp(WindowPropFlags_t prop_flag) = 0;
        // Set a property flag
        virtual bool WindowSetProp(WindowPropFlags_t prop_flag, bool val) = 0;
        // Get all properties
        virtual WindowProps_t GetWindowProps() = 0;

        virtual bool Minimise() = 0;
        virtual bool Restore() = 0;
        virtual bool Maximise() = 0;
        virtual bool ToggleFullscreen() = 0;
        virtual bool ToggleBorderlessWindowed() = 0;
    };

} // namespace Myriad

#endif
