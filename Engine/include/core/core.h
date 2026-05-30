#ifndef __CORE_H_
#define __CORE_H_

#include "core/config.h"

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

    class Vector2
    {
      public:
        float x;
        float y;
        Vector2(float x, float y) : x(x), y(y) {}
        Vector2() : Vector2(0.0f, 0.0f) {};
    };

    template <typename V, typename D> class Rect
    {
      public:
        V pos;
        V size;
        V anchor;
        Rect() : pos(), size(), anchor() {}
        Rect(D sizex, D sizey) : pos(), size(sizex, sizey), anchor() {}
        Rect(V pos, V size) : pos(pos), size(size), anchor() {}
        Rect(V pos, V size, V anchor) : pos(pos), size(size), anchor(anchor) {}
        inline D Top() { return pos.y - anchor.y; }
        inline D Bottom() { return pos.y - anchor.y + size.y; }
        inline D Left() { return pos.x - anchor.x; }
        inline D Right() { return pos.x - anchor.x + size.x; }
    };

    class Rect2D : public Rect<Vector2, float>
    {
      public:
        Rect2D() : Rect() {}
        Rect2D(Vector2 pos, Vector2 size) : Rect(pos, size) {}
        Rect2D(Vector2 pos, Vector2 size, Vector2 anchor)
            : Rect(pos, size, anchor)
        {
        }
    };

    class IVector2
    {
      public:
        int x;
        int y;
        IVector2(int x, int y) : x(x), y(y) {}
        IVector2() : IVector2(0, 0) {}
    };

    class IRect2D : public Rect<IVector2, int>
    {
    };

    class Vector3
    {
      public:
        float x;
        float y;
        float z;
    };

    class Quaternion
    {
      public:
        float x;
        float y;
        float z;
        float w;
    };
} // namespace Myriad

#endif
