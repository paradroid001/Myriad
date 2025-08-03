#ifndef hgjhgjh
#define hgjhgjh

#include "myriad.h"
#include "TestGameObjectBase.h"

// using TGOMAllocator_t = Myriad::TypeAllocatorDynamic<TestGameObjectBase>;

class TestGameObjectManager : public Myriad::MyrObjectManager<TestGameObjectBase, Myriad::TypeAllocatorDynamic<TestGameObjectBase>>
{

public:
  TestGameObjectManager();
  ~TestGameObjectManager();

  // Iterator:
  auto begin() noexcept
  {
    return p_allocator_->begin();
  }
  auto end() noexcept
  {
    return p_allocator_->end();
  }

  void OnObjectCreate(MYR_ID_t id, void *ptr) override;

  void DestroyObjectById(MYR_ID_t id) override;
  TestGameObjectBase *GetObject(MYR_ID_t id) override;
  bool AddChild(MYR_ID_t parent, MYR_ID_t child);
};

#endif
