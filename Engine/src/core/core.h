#ifndef __CORE_H_
#define __CORE_H_

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

// Bitshift macro
#define BIT(x) (1 << x)

namespace Myriad
{
    /**
     * A class for representation of colours as
     * four 8 bit integer values, or 32 bit RGBA
     */
    class MyrColour
    {
      public:
        unsigned char r;
        unsigned char g;
        unsigned char b;
        unsigned char a;
    };
} // namespace Myriad

#endif