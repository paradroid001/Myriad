#ifndef MYRIAD_CORE_OBJECT_MYRCOMPONENT_H
#define MYRIAD_CORE_OBJECT_MYRCOMPONENT_H

#include "core/IEnableable.h"
#include "core/core.h"

namespace Myriad
{
    class MyrObject; // fwd
    class MYR_API MyrComponent : public IEnableable
    {
      public:
        MyrComponent() : owner_(nullptr) {}
        //Commenting out this constructor, nobody wants to
        //create a component this way.
        //MyrComponent(MyrObject *owner);

        virtual ~MyrComponent() {}
        /// @brief For now, you have to init component with the
        ///        myrobject it is being attached to before you add it.
        /// @param owner
        /// @return
        virtual bool InitComponent(MyrObject *owner);
        virtual bool ReleaseComponent() = 0;
        virtual MyrObject *GetOwner() const;
        // IEnableable
        virtual void Enable() override;
        virtual void Disable() override;

      protected:
        MyrObject *owner_;
        virtual void OnEnable() override;
        virtual void OnDisable() override;
    };
} // namespace Myriad

#endif
