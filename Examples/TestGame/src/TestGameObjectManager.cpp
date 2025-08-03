#include "TestGameObjectManager.h"

TestGameObjectManager::TestGameObjectManager() : MyrObjectManager<TestGameObjectBase, Myriad::TypeAllocatorDynamic<TestGameObjectBase>>()
{
  p_allocator_ = new Myriad::TypeAllocatorDynamic<TestGameObjectBase>(100, this);
}
TestGameObjectManager::~TestGameObjectManager()
{
  delete p_allocator_;
}

void TestGameObjectManager::DestroyObjectById(MYR_ID_t id)
{
  MYR_INFO("TestGameObjectManager destroy by id not implemented");
}
TestGameObjectBase *TestGameObjectManager::GetObject(MYR_ID_t id)
{
  return nullptr;
}
bool TestGameObjectManager::AddChild(MYR_ID_t parent, MYR_ID_t child)
{
  MYR_INFO("TestGameObjectManager AddChild is not implemented");
  return false;
}

void TestGameObjectManager::OnObjectCreate(MYR_ID_t id, void *ptr)
{
  MYR_TRACE("Test game object manager create an object, id: {0} obj->id {1}", id, static_cast<TestGameObjectBase *>(ptr)->GetId());
}
