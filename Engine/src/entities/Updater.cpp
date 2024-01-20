#include "entities/Updater.h"
//#include "core/core.h"
#include "io/Log.h"
namespace Myriad
{
    Updater::Updater() : Component()
    {
        MYR_CORE_TRACE("Updater Constructor");
    }
    Updater::~Updater()
    {
        MYR_CORE_TRACE("Updater Destructor");
    }
    void Updater::Update(float dt)
    {
        //override me.
    }
}