#ifndef MYRIAD_GAME_GAMEOBJECTMANAGER_H
#define MYRIAD_GAME_GAMEOBJECTMANAGER_H

#include "core/core.h"
#include "core/config.h"
#include "core/MyrObjectManager.h"
#include "core/MyrAlloc.h"
#include "io/MyrLogging.h"
#include <unordered_map>
#include <vector>

namespace Myriad
{
  /*
  template <typename T>
  class MYR_API GameObjectRecord
  {
  protected:
    T *p_object_;
    std::vector<T *> children_;

  public:
    GameObjectRecord() : p_object_(nullptr)
    {
      Initialise();
    }
    ~GameObjectRecord()
    {
      if (p_object_ != nullptr)
        delete p_object_;
    }
    void Initialise()
    {
      p_object_ = nullptr;
      children_.clear();
    }
  };

  template <typename T>
  class MYR_API GameObjectManager : public MyrObjectManager
  {
  protected:
    // The object map
    std::unordered_map<MYR_ID_t, T *> object_map_;
    // The next assignable id
    MYR_ID_t next_id_;

  public:
    GameObjectManager() : next_id_(0)
    {
      MYR_CORE_TRACE("GameObjectManager constructor");
    }
    ~GameObjectManager()
    {
      MYR_CORE_TRACE("GameObjectManager destructor");
    }

    MYR_ID_t CreateObject() override
    {
      if (next_id_ < MYR_OBJECT_INVALID)
      {
        T *g = new T(next_id_);
        g->manager_ = this;
        object_map_[next_id_] = g;
        next_id_++;
        return g->GetID();
      }
      return MYR_OBJECT_INVALID;
    }
    // void Destroy(MyrGameObject *object) override;
    void DestroyObjectById(MYR_ID_t id) override
    {
      if (object_map_.find(id) != object_map_.end())
      {
        T *pg = object_map_[id];
        if (pg != nullptr)
        {
          delete object_map_[id];
          object_map_[id] = nullptr;
        }
      }
    }
    T *GetObject(MYR_ID_t id) override
    {
      // find the object
      if (object_map_.find(id) != object_map_.end())
      {
        return object_map_[id];
      }
      return nullptr;
    }

    bool AddChild(MYR_ID_t parent, MYR_ID_t child) override
    {
      // if the parent exists and the child exists
      // and the child isn't already part of the parent
      return false;
    }
  };
  */
}

#endif
