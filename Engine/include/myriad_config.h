#ifndef _MYRIAD_CONFIG_H_
#define _MYRIAD_CONFIG_H_

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

#include <cstdint>
#include <string>

namespace Myriad
{
    /* Base ID type*/
    typedef uint32_t MYR_ID_t;
    // Assets are MYR_ID_t: so that's
    // things like textures, fonts, shaders, audio, etc.
    typedef MYR_ID_t AssetID_t;

    const std::string DEFAULT_RESOURCE_BASE_PATH = "shared/res";
    const std::string DEFAULT_CONFIG_PATH =
        DEFAULT_RESOURCE_BASE_PATH + "/config.s8";

    // Font Limits
    const MYR_ID_t MAX_FONTS = 32;
    const MYR_ID_t FONTHANDLE_INVALID = MAX_FONTS + 1;
    // Texture Limits
    const MYR_ID_t MAX_TEXTURES = 500;
    const MYR_ID_t TEXHANDLE_INVALID = MAX_TEXTURES + 1;
} // namespace Myriad

#endif // _MYRIAD_CONFIG_H_
