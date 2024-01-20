#ifndef __UPDATER_H_
#define __UPDATER_H_

#include "core/core.h"
#include "core/IUpdateable.h"
#include "core/Component.h"

namespace Myriad
{
    class MYR_API Updater : public Component, public IUpdateable
    {
        public:
            Updater();
            virtual ~Updater();
            virtual void Update(float dt) override;
    };
}

#endif