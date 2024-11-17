#include "core/component/MyrComponent.h"
#include "core/object/MyrObject.h"

namespace Myriad
{
    /*
    MyrComponent::MyrComponent(MyrObject *owner) : MyrComponent()
    {
        InitComponent(owner);
    }
    */

    bool MyrComponent::InitComponent(MyrObject *owner)
    {
        assert(owner_ == nullptr);
        owner_ = owner;
        return true;
    }
    bool MyrComponent::ReleaseComponent()
    {
        // TODO.
        return true;
    }

    MyrObject *MyrComponent::GetOwner() const { return owner_; }
    void MyrComponent::Enable() {}
    void MyrComponent::Disable() {}
    void MyrComponent::OnEnable() {}
    void MyrComponent::OnDisable() {}
} // namespace Myriad
