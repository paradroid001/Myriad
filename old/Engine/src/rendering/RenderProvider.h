#ifndef MYRIAD_RENDERING_RENDERPROVIDER_H
#define MYRIAD_RENDERING_RENDERPROVIDER_H

#include "asset/Font.h"      //for fonts
#include "asset/Texture2D.h" //for texture2d
#include "core/Provider.h"
#include "core/core.h" //for types

namespace Myriad
{
    class RenderProvider : public Provider
    {
      public:
        virtual ~RenderProvider();

        // Provider
        virtual bool Shutdown() = 0;

        // Our Methods
        virtual bool Init() = 0;
        // Set BG Colour
        virtual void ClearBackground(MyrColour colour) = 0;
        // Framebuffer Start Drawing
        virtual void BeginDrawing() = 0;
        virtual void EndDrawing() = 0;

        virtual void DrawCircle(Vector2 pos, float radius,
                                MyrColour colour) = 0;

        virtual void DrawTexture(Texture2D tex, Vector2 pos,
                                 MyrColour colour) = 0;

        virtual void DrawText(Font font, std::string text, Vector2 pos,
                              int size, MyrColour colour) = 0;

        /*
        // Drawing-related functions
        void ClearBackground(Color color);                          // Set
        background color (framebuffer clear color) void BeginDrawing(void); //
        Setup canvas (framebuffer) to start drawing void EndDrawing(void); //
        End canvas drawing and swap buffers (double buffering) void
        BeginMode2D(Camera2D camera);                          // Begin 2D mode
        with custom camera (2D) void EndMode2D(void); // Ends 2D mode with
        custom camera void BeginMode3D(Camera3D camera); // Begin 3D mode with
        custom camera (3D) void EndMode3D(void); // Ends 3D mode and returns to
        default 2D orthographic mode void BeginTextureMode(RenderTexture2D
        target);
        // Begin drawing to render texture void EndTextureMode(void); // Ends
        drawing to render texture void BeginShaderMode(Shader shader); // Begin
        custom shader drawing void EndShaderMode(void); // End custom shader
        drawing (use default shader) void BeginBlendMode(int mode); // Begin
        blending mode (alpha, additive, multiplied, subtract, custom) void
        EndBlendMode(void); // End blending mode (reset to default: alpha
        blending) void BeginScissorMode(int x, int y, int width, int height); //
        Begin scissor mode (define screen area for following drawing) void
        EndScissorMode(void); // End scissor mode void
        BeginVrStereoMode(VrStereoConfig config);              // Begin stereo
        rendering (requires VR simulator) void EndVrStereoMode(void);
        // End stereo rendering (requires VR simulator)
        */
    };
} // namespace Myriad

#endif
