#include "Allocator.h"
#include "Handle.h"
#include <iostream>
#include <string>
#include <vector>

class TestClassBase
{
public:
  virtual void Print() = 0;
  virtual ~TestClassBase() {}
};

class TestClass : public TestClassBase
{
public:
  int health;
  int level;
  std::string name;
  TestClass() : health(0), level(0), name("Unknown") {}
  TestClass(int h, int l, std::string n) : health(h), level(l), name(n){};
  ~TestClass() { std::cout << "I am the Testclass Destructor" << std::endl; }
  void Print() override
  {
    std::cout << name << " has " << health << " hitpoints and is level " << level << std::endl;
  }
};

class TestClass2 : public TestClassBase
{
public:
  std::string message;
  TestClass2(std::string m) : message(m) {}
  ~TestClass2() { std::cout << "I am the Testclass2 Destructor" << std::endl; }
  void Print() override
  {
    std::cout << message << std::endl;
  }
};

void HandlePrinter(TestClassBase *h)
{
  h->Print();
}

int main(int argc, char **argv)
{
  std::cout << "Hello, World!" << std::endl;
  Allocator *allocator = new Allocator();
  Handle<TestClass> h1 = allocator->Alloc<TestClass>();
  h1.Get()->Print();
  Handle<TestClass> h2 = allocator->Alloc<TestClass>(12, 10, "Thrall");
  h2->Print();
  Handle<TestClass2> h3 = allocator->Alloc<TestClass2>("Hello, World!");
  (*h3).Print();
  auto h4 = allocator->Alloc<TestClass2>("Goodbye, World!");
  h4->Print();

  std::vector<TestClassBase *> testptrs;
  testptrs.push_back(h1.Get());
  testptrs.push_back(h2.Get());
  testptrs.push_back(h3.Get());
  testptrs.push_back(h4.Get());

  std::cout << "HandlePrinter Starting" << std::endl;
  for (const auto &tcb : testptrs)
  {
    tcb->Print();
  }
  std::cout << "HandlePrinter Ending" << std::endl;

  std::cout << "Doing the same with ids." << std::endl;
  // Can we do the same thing with just uint16s?
  std::vector<uint16_t> testids;
  testids.push_back(h1.index);
  testids.push_back(h2.index);
  testids.push_back(h3.index);
  testids.push_back(h4.index);

  for (const auto &i : testids)
  {
    // If we know the allocator we should be able to get the
    // object...
    TestClassBase *p_tcb = allocator->GetPtr<TestClassBase>(i);
    p_tcb->Print();
  }

  std::cout << "Ending id test" << std::endl;

  delete allocator;
  return 0;
}
