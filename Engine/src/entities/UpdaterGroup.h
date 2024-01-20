#ifndef __UPDATERGROUP_H_
#define __UPDATERGROUP_H_

#include "core/core.h"
#include "core/GroupList.h"
#include "entities/Updater.h"
#include "core/IUpdateable.h"

#include <list>
using std::list;

namespace Myriad
{
    class MYR_API UpdaterGroup : public GroupList<Updater>, public IUpdateable
    {
        public:
            UpdaterGroup();
            virtual ~UpdaterGroup();
            // UpdaterGroup specifically can Update
            virtual void Update(float dt) override;
    };
}

#endif