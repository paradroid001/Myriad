#ifndef __RENDERER_H_
#define __RENDERER_H_

#include "Component.h"

namespace Myriad
{
    // fwd declare transform;
    class Transform;

    struct RendererData : ComponentData
    {
    };

    class Renderer : public Myriad::Component
    {
      public:
        virtual void Draw(Transform t);
        ComponentData *Data() { return &_internalData; }

      protected:
        class RendererData _internalData;
    };
} // namespace Myriad

#endif