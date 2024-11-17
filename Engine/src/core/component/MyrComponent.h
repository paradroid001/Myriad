#ifndef MYRIAD_CORE_OBJECT_MYRCOMPONENT_H
#define MYRIAD_CORE_OBJECT_MYRCOMPONENT_H

#include "core/IEnableable.h"
#include "core/core.h"
#include "io/Log.h"
#include <cstdint>

namespace Myriad
{
    class MyrObject; // fwd

    class MYR_API MyrComponent : public IEnableable
    {
      public:
        MyrComponent() : owner_(nullptr)
        {
            MYR_CORE_INFO("Yes, MyrComponent constructor is being hit");
        }
        // Commenting out this constructor, nobody wants to
        // create a component this way.
        // MyrComponent(MyrObject *owner);

        virtual ~MyrComponent() {}
        /// @brief For now, you have to init component with the
        ///        myrobject it is being attached to before you add it.
        /// @param owner
        /// @return
        virtual bool InitComponent(MyrObject *owner);
        virtual bool ReleaseComponent();
        virtual MyrObject *GetOwner() const;
        // IEnableable
        virtual void Enable() override;
        virtual void Disable() override;

      protected:
        MyrObject *owner_;
        virtual void OnEnable() override;
        virtual void OnDisable() override;
    };

    template <class T> class MyrComponentBase : public MyrComponent
    {
      public:
        static uint16_t type;
        int GetType() const { return T::type; }
    };

    static uint16_t nextComponentType = 0;
    template <typename T>
    uint16_t MyrComponentBase<T>::type(nextComponentType++);

} // namespace Myriad

#endif
