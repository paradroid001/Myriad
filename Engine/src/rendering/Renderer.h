#ifndef __RENDERER_H_
#define __RENDERER_H_

#include "core/Component.h"
#include "core/Types2D.h"
#include "core/core.h"

namespace Myriad
{
    // fwd declare transform;
    struct TransformData;

    struct RendererData : ComponentData
    {
    };

    class MYR_API Renderer : public Myriad::Component
    {
      public:
        Renderer() : Myriad::Component()
        {
            // TODO this is unsatisfactory because
            // I am loathe to set renderdaya here,
            // since inheriting classes will then
            // reset over the top. See the other note
            // about making SetComponentData pure
            // protected virtual, and then inheriting classes
            // are forced to implement and it gives the most
            // derived implementation during construction.
            SetComponentData(&_renderData);
        };
        virtual ~Renderer();
        virtual void Draw(TransformData t);
        // ComponentData *Data() { return &_internalData; }
        static void MyrDrawCircle(float x, float y, float rad,
                                  Myriad::MyrColour colour);
        static void MyrDrawTriangle(Myriad::Vector2 p1, Myriad::Vector2 p2,
                                    Myriad::Vector2 p3,
                                    Myriad::MyrColour colour);
        static void MyrDrawText(const char *text, float x, float y,
                                float lineheight, Myriad::MyrColour colour);

      protected:
        RendererData _renderData;
    };
} // namespace Myriad

#endif