#include "core/Transform.h"

#include "RendererGroup.h"
#include "io/Log.h"

#include <cstdlib>

namespace Myriad
{
    RendererGroup::RendererGroup()
    {
        // Constructor
        MYR_CORE_TRACE("Renderergroup constructor");
        iterating = false;
    }
    RendererGroup::~RendererGroup()
    {
        // Destructor
        MYR_CORE_TRACE("Renderergroup destructor");
    }
    void RendererGroup::add(Renderer &renderer)
    {
        m_rendererList.push_back(&renderer);
    }
    void RendererGroup::remove(Renderer &renderer)
    {
        m_rendererList.remove(&renderer);
    }
    int RendererGroup::count() { return m_rendererList.size(); }

    Renderer *RendererGroup::iterate()
    {
        if (!iterating)
        {
            m_iterator = m_rendererList.begin();
            iterating = true;
        }
        else
        {
            ++m_iterator;
        }

        if (m_iterator == m_rendererList.end())
        {
            iterating = false;
            return NULL;
        }
        return (Renderer *)*m_iterator;
    }

    void RendererGroup::draw()
    {
        Renderer *current;
        while ((current = iterate()) != NULL)
        {
            TransformData tdata;
            // TODO this is just a test
            tdata.position = {(float)(rand() % 100), (float)(rand() % 100),
                              (float)(rand() % 100)};
            current->Draw(tdata);
        }
    }
} // namespace Myriad