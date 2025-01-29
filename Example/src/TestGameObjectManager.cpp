#include "TestGameObjectManager.h"
#include "core/MyrObjectManager.h"

TestGameObjectManager::TestGameObjectManager() : MyrObjectManager<TestGameObjectBase, Myriad::TypeAllocatorDynamic<TestGameObjectBase>>()
{
  p_allocator = new Myriad::TypeAllocatorDynamic<TestGameObjectBase>(100);
}
TestGameObjectManager::~TestGameObjectManager()
{
  delete p_allocator;
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
