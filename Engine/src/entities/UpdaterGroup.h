#ifndef __UPDATERGROUP_H_
#define __UPDATERGROUP_H_

#include "core/IGroup.h"
#include "core/core.h"
#include "core/IUpdateable.h"

#include <list>
using std::list;

namespace Myriad
{
    class MYR_API UpdaterGroup : public IGroup<IUpdateable>, public IUpdateable
    {
        public:
            UpdaterGroup();
            virtual ~UpdaterGroup();
            virtual void Add(IUpdateable& u) override;
            virtual void Remove(IUpdateable& u) override;
            virtual int Count() override;
            virtual IUpdateable* Iterate() override;

            virtual void Update(float dt) override;
        protected:
            std::list<IUpdateable *> m_updaterList;
            std::list<IUpdateable *>::iterator m_iterator;
            bool iterating;
    };
}

#endif