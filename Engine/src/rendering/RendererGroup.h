#ifndef __RENDERERGROUP_H_
#define __RENDERERGROUP_H_

#include "core/core.h"
#include "rendering/Renderer.h"
#include "core/GroupList.h"

#include <list>
using std::list;

namespace Myriad
{
    class MYR_API RendererGroup : public GroupList<Renderer>, public IDrawable
    {
      public:
        RendererGroup();
        virtual ~RendererGroup();
        // RendererGroup specifically can Draw.
        virtual void Draw() override;
    };
} // namespace Myriad

#endif