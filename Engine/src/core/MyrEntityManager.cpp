#include "core/MyrEntityManager.h"
#include "io/MyrLogging.h"
namespace Myriad
{
  // make sure the static var is defined
  MyrEntityManager *MyrEntityManager::s_this_ = nullptr;
  void MyrEntityManager::SetEntityManager(MyrEntityManager *em)
  {
    s_this_ = em;
  }

  MyrEntityManager::~MyrEntityManager()
  {
    // clean up each descriptor
    // std::unordered_map<MyrComponentType, MyrComponentsDescriptor>::iterator it;
    for (auto it = components_.begin(); it != components_.end(); ++it)
    {

      // for (auto &[k, v] : components_)
      //{
      MYR_CORE_TRACE("Cleaning up component type {0}", static_cast<int>(it->first));
      if (it->second != nullptr)
      {
        delete it->second;
      }
    }
  }

  MyrEntity *MyrEntityManager::GetEntity(MYR_ID_t entity_id)
  {
    // find the entity
    MyrEntity *entities = s_this_->entities_;
    for (MYR_ID_t i = 0; i < MYRIAD_MAX_ENTITIES; i++)
    {
      if (entities[i].GetID() == entity_id)
        return &(entities[i]);
    }
    return nullptr;
  }

  bool MyrEntityManager::AddComponent(MYR_ID_t entity_id, MYR_ID_t component_id)
  {
    /*
    MyrEntity* p_e = GetEntity(entity_id);
    if (p_e != nullptr)
    {
      //we found the entity. good. Add the component.
    }
    */
    MYR_WARN("AddComponent(eid, cid) not implemented. This will take some work. Do we really need this?");
    return false;
  }

  bool MyrEntityManager::AddComponent(const MyrEntity entity, MyrComponent &component)
  {
    // TODO: Checks -
    // Make sure component isn't already on this entity (do nothing?)
    // Is component on another entity? Are we moving a component between entities?
    component.SetEID(entity.GetID());
    return true;
  }

}
