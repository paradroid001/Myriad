#ifndef __RENDERERGROUP_H_
#define __RENDERERGROUP_H_

#include "core/IGroup.h"
#include "core/core.h"
#include "rendering/Renderer.h"

#include <list>
using std::list;

namespace Myriad
{
    class MYR_API RendererGroup : public IGroup<Renderer>
    {
      public:
        RendererGroup();
        virtual ~RendererGroup();
        virtual void add(Renderer &r) override;
        virtual void remove(Renderer &r) override;
        virtual int count() override;
        virtual Renderer *iterate() override;

        // RendererGroup specifically can Draw.
        virtual void draw();

      protected:
        std::list<Renderer *> m_rendererList;
        std::list<Renderer *>::iterator m_iterator;
        bool iterating;
    };
} // namespace Myriad

#endif