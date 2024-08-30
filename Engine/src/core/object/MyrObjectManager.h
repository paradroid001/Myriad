#ifndef MYRIAD_CORE_OBJECT_MYROBJECTMANAGER_H
#define MYRIAD_CORE_OBJECT_MYROBJECTMANAGER_H

#include "core/IService.h"
#include "core/MyrSingleton.h"
#include "core/core.h"

namespace Myriad
{
    class MyrObject; // fwd declare

    class MYR_API MyrObjectManager : public IService,
                                     public MyrSingleton<MyrObjectManager>
    {
        friend MyrObject;
        // This using line prevents errors finding the
        // myrsingleton constructor
        using MyrSingleton<MyrObjectManager>::MyrSingleton;

      protected:
        bool WhenStartService() override;
        bool WhenStopService() override;

        void CreateObject();
        void DestroyObject();
        void AddChild();
        void RemoveChild();
        void ReparentChild();

      public:
        ~MyrObjectManager();
    };
} // namespace Myriad
#endif
