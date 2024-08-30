#pragma once

#include <string>
#include <iostream>

inline int type_id_seq = 0;
template <typename T>
inline const int type_id = type_id_seq++;

class Fruit
{
protected:
  bool has_seeds;
  std::string name;

public:
  Fruit(std::string name, bool seeds) : has_seeds(seeds), name(name) {}
  virtual ~Fruit() {}
  virtual void Print()
  {
    std::cout << "Name: " << name << ", has seeds: " << has_seeds << std::endl;
  }
};

class Apple : public Fruit
{
public:
  Apple() : Fruit("Apple", true) {}
};

class Orange : public Fruit
{
public:
  Orange() : Fruit("Orange", true) {}
};

class Banana : public Fruit
{
public:
  Banana() : Fruit("Banana", false) {}
};
