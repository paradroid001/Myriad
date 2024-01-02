#ifndef __RENDERERGROUP_H_
#define __RENDERERGROUP_H_

#include "IGroup.h"
#include "Renderer.h"
#include "core.h"

#include <list>
using std::list;

namespace Myriad
{
    class MYR_API RendererGroup : public IGroup<Renderer *>
    {
      public:
        RendererGroup();
        virtual ~RendererGroup();
        virtual void Add(Renderer *r) override;
        virtual void Remove(Renderer *r) override;
        virtual int Count() override;
        virtual Renderer *Iterate() override;

        // RendererGroup specifically can Draw.
        virtual void Draw();

      protected:
        std::list<Renderer *> m_rendererList;
        std::list<Renderer *>::iterator m_iterator;
        bool iterating;
    };
} // namespace Myriad

#endif