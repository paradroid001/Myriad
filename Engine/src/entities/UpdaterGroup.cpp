#include "entities/Updater.h"
#include "entities/UpdaterGroup.h"
#include "io/Log.h"


namespace Myriad
{
    UpdaterGroup::UpdaterGroup() : GroupList<Updater>()
    {
        MYR_CORE_TRACE("Updatergroup constructor");
    }
    UpdaterGroup::~UpdaterGroup()
    {
        MYR_CORE_TRACE("Updatergroup destructor");
    }
    
    void UpdaterGroup::Update(float dt)
    {
        Updater* current;
        while ((current = Iterate()) != NULL)
        {
            current->Update(dt);
        }
    }
}