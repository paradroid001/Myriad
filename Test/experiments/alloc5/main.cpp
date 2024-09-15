#include "Allocator.h"
#include <vector>

class Fruit
{
private:
  bool seeds;
  std::string colour;
  std::string name;

public:
  Fruit() {}
  Fruit(bool s, std::string col, std::string n) : seeds(s), colour(col), name(n)
  {
    std::cout << "Alloced a fruit: " << name << " (" << this << ")" << std::endl;
  }
  virtual ~Fruit()
  {
    std::cout << "Deleted a fruit: " << name << " (" << this << ")" << std::endl;
  }

  void Describe()
  {
    std::cout << name << ": Seeds=" << seeds << ", Colour=" << colour << std::endl;
  }
};

class Orange : public Fruit
{
public:
  Orange() : Fruit(4, "Orange", "Orange") {}
  ~Orange() { std::cout << "Orange I finished?" << std::endl; }
};
class Kiwi : public Fruit
{
public:
  Kiwi() : Fruit(50, "Green", "Kiwi") {}
  ~Kiwi() { std::cout << "I used to be a kiwi" << std::endl; }
};

class Pencil
{
  void Describe()
  {
    std::cout << "I am a pencil" << std::endl;
  }
};

int main(int argc, char **argv)
{
  Allocator<Fruit> *p_fruit_allocator = new Allocator<Fruit>(uint32_t(5)); // 100 fruit.

  std::vector<Handle_T> fruit_handles;

  Handle_T f_handle = p_fruit_allocator->Alloc(false, "Yellow", "Banana");
  Handle_T orange_handle = p_fruit_allocator->Alloc<Orange>();
  Handle_T kiwi_handle = p_fruit_allocator->Alloc<Kiwi>();
  fruit_handles.push_back(f_handle);
  fruit_handles.push_back(orange_handle);
  fruit_handles.push_back(kiwi_handle);

  // So this shouldn't work.
  // (good, it doesn't)
  // Handle_T pencil_handle = p_fruit_allocator->Alloc<Pencil>();
  // fruit_handles.push_back(pencil_handle);

  std::cout << std::endl
            << "Describe Phase" << std::endl;
  for (auto &h : fruit_handles)
  {
    p_fruit_allocator->Get(h)->Describe();
  }

  // delete fruit out.
  p_fruit_allocator->Free(f_handle);
  // Alloc a fruit back in, it should occupy the same pos.
  p_fruit_allocator->Alloc<Kiwi>();
  // Now alloc another fruit, this should be a new mem pos.
  p_fruit_allocator->Alloc<Orange>();

  std::cout << std::endl
            << "Cleanup Phase" << std::endl;
  p_fruit_allocator->PrintStats();
  delete p_fruit_allocator;
  return 0;
}
