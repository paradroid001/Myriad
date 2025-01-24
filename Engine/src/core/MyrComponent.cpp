#include "core/MyrComponent.h"
#include "io/MyrLogging.h"

namespace Myriad
{
  MyrComponent::MyrComponent() : id_(MYRIAD_COMPONENT_INVALID_ID), entity_id_(MYRIAD_ENTITY_INVALID_ID), type_(MyrComponentType::NULL_COMPONENT)
  {
    MYR_CORE_TRACE("MyrComponent Constructor");
  }
  MyrComponent::~MyrComponent()
  {
    MYR_CORE_TRACE("MyrComponent Destructor");
  };
}
