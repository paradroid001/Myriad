#ifndef _MYRIAD_ENGINE_CONFIG_H_
#define _MYRIAD_ENGINE_CONFIG_H_

/* PLATFORM SPECIFIC DEFINITIONS */

#ifdef MYR_PLATFORM_WINDOWS
    #ifdef MYR_BUILD_DLL
        #define MYR_API __declspec(dllexport)
    #else
        #define MYR_API __declspec(dllimport)
    #endif
#else
    // define as empty for all other platforms
    #define MYR_API
#endif

#if defined(_WIN32)
    #define NOGDI  // All GDI defines and routines
    #define NOUSER // All USER defines and routines
#endif

// Myriad Renderer
#define RENDERER_NONE 0
#define RENDERER_RAYLIB 1
// 0 = none
// 1 = raylib
#define MYRIAD_RENDERER RENDERER_RAYLIB
// Max entities at once
#define MYRIAD_MAX_ENTITIES 100
// Max components at once
#define MYRIAD_MAX_COMPONENTS 100
#define MYRIAD_MAX_COMPONENTS_PER_ENTITY 16

#define MYRIAD_INVALID_ID 0xFFFFFFFF - 1

#include <cstdint> // For fixed-width integer types

namespace Myriad
{
    /* Base ID type*/
    typedef uint32_t MYR_ID_t;
    // Assets are MYR_ID_t: so that's
    // things like textures, fonts, shaders, audio, etc.
    typedef MYR_ID_t AssetID_t;

    // Font Limits
    const MYR_ID_t MAX_FONTS = 32;
    const MYR_ID_t FONTHANDLE_INVALID = MAX_FONTS + 1;
    // Texture Limits
    const MYR_ID_t MAX_TEXTURES = 500;
    const MYR_ID_t TEXHANDLE_INVALID = MAX_TEXTURES + 1;

} // namespace Myriad
#endif
