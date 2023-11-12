#ifndef __RENDERER_H_
#define __RENDERER_H_

#include "Component.h"

namespace Myriad
{
    // fwd declare transform;
    struct TransformData;

    struct RendererData : ComponentData
    {
    };

    class Renderer : public Myriad::Component
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
        virtual void Draw(TransformData t);
        // ComponentData *Data() { return &_internalData; }

      protected:
        RendererData _renderData;
    };
} // namespace Myriad

#endif