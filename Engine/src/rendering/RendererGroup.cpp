#include "core/Transform.h"

#include "RendererGroup.h"
#include "core/GameObject.h" //for the cast
#include "io/Log.h"

#include <cstdlib>

namespace Myriad
{
    RendererGroup::RendererGroup() : GroupList<Renderer>()
    {
        // Constructor
        MYR_CORE_TRACE("Renderergroup constructor");
    }
    RendererGroup::~RendererGroup()
    {
        // Destructor
        MYR_CORE_TRACE("Renderergroup destructor");
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