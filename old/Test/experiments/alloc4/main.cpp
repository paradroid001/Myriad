#include <iostream>
#include "Allocator.h"
#include "Handle.h"

class Fruit
{
protected:
  std::string name_;

public:
  Fruit(std::string n) : name_(n) {}
  virtual ~Fruit()
  {
    std::cout << "Destructing Fruit" << std::endl;
  }
  std::string &GetName() { return name_; }
};

class Orange : public Fruit
{
public:
  Orange() : Fruit("Orange") {}
  ~Orange()
  {
    std::cout << "Destructing ORANGE" << std::endl;
  }
};

void PrintFruitAt(Allocator<Fruit> *allocator, unsigned int index)
{
  Fruit *f = allocator->get<Fruit>(index);
  if (f != nullptr)
  {
    std::cout << "Got [" << index << "]: " << f->GetName() << std::endl;
  }
  else
  {
    std::cout << "Got [" << index << "]: NULL" << std::endl;
  }
}

int main(int argc, char **argv)
{
  Allocator<Fruit> *fruit_allocator = new Allocator<Fruit>();
  Allocator<int> *int_allocator = new Allocator<int>();

  // Allocator<BaseAlloc> *void_allocator = new Allocator<BaseAlloc>();

  unsigned int h1 = fruit_allocator->Alloc<Fruit>("Basic Fruit");
  PrintFruitAt(fruit_allocator, h1);
  unsigned int h2 = fruit_allocator->Alloc<Orange>();
  PrintFruitAt(fruit_allocator, h2);

  // Into a new allocator
  unsigned int h3 = int_allocator->Alloc<int>(7);

  // unsigned int h4 = void_allocator->Alloc<Fruit>("Void* fruit");
  // void *fruit = void_allocator->get<Fruit *>(h4);
  // std::cout << "The rescued fruit was " << (static_cast<Fruit *>(fruit))->GetName() << std::endl;

  delete fruit_allocator;
  delete int_allocator;
  // delete void_allocator; // this will probably break.

  return 0;
}
