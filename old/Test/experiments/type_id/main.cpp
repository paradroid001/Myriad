#include <iostream>
#include <unordered_map>
#include <vector>
#include "types.h"

class FruitBowl
{
public:
  std::unordered_map<int, std::vector<Fruit *> *> fruit;

  template <typename T>
  void AddFruit(Fruit *pfruit)
  {
    int type = type_id<T>;
    if (fruit.find(type) == fruit.end())
    {
      fruit[type] = new std::vector<Fruit *>();
    }
    fruit[type]->push_back(pfruit);
  }

  template <typename T>
  std::vector<Fruit *> *GetFruit()
  {
    int type = type_id<T>;
    if (fruit.find(type) != fruit.end())
    {
      return fruit[type];
    }
    return nullptr;
  }
};

int main(int argc, char **argv)
{
  FruitBowl bowl;
  bowl.AddFruit<Apple>(new Apple());
  bowl.AddFruit<Banana>(new Banana());
  bowl.AddFruit<Banana>(new Banana());
  bowl.AddFruit<Orange>(new Orange());
  bowl.AddFruit<Apple>(new Apple());

  std::cout << bowl.GetFruit<Orange>()->size() << " Oranges." << std::endl;
  std::cout << bowl.GetFruit<Apple>()->size() << " Apples." << std::endl;
  std::cout << bowl.GetFruit<Banana>()->size() << " Bananas." << std::endl;
  for (auto f : *(bowl.GetFruit<Apple>()))
  {
    f->Print();
  }
  for (auto f : *(bowl.GetFruit<Orange>()))
  {
    f->Print();
  }
  return 0;
}
