#include "entities/UpdaterGroup.h"
#include "core/IUpdateable.h"

namespace Myriad
{
    UpdaterGroup::UpdaterGroup()
    {
        iterating = false;
    }
    UpdaterGroup::~UpdaterGroup()
    {

    }
    void UpdaterGroup::Add(IUpdateable& updateable)
    {

    }
    void UpdaterGroup::Remove(IUpdateable& updateable)
    {

    }
    int UpdaterGroup::Count()
    {
        return 0;
    }
    IUpdateable* UpdaterGroup::Iterate()
    {
        return NULL;
    }
    void UpdaterGroup::Update(float dt)
    {

    }
}