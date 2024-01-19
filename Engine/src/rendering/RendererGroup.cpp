#include "core/Transform.h"

#include "RendererGroup.h"
#include "core/GameObject.h" //for the cast
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
    void RendererGroup::Add(Renderer &renderer)
    {
        m_rendererList.push_back(&renderer);
    }
    void RendererGroup::Remove(Renderer &renderer)
    {
        m_rendererList.remove(&renderer);
    }
    int RendererGroup::Count() { return m_rendererList.size(); }

    Renderer *RendererGroup::Iterate()
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

    void RendererGroup::Draw()
    {
        Renderer *current;
        while ((current = Iterate()) != NULL)
        {
            //TransformData tdata;
            // TODO this is just a test
            //tdata.position = {(float)(rand() % 100), (float)(rand() % 100),
            //                  (float)(rand() % 100)};

            current->Draw();//tdata);

            // What if we tried to get the game object to handle it?
            // This doesn't work because it returns an interface or something...
            //((GameObject *)current->GetGameObject())->Draw();
        }
    }
} // namespace Myriad