#ifndef _TASKS_H_
#define _TASKS_H_

#include <iostream>

class IterableState
{
public:
  virtual void Iterate() = 0;
  virtual ~IterableState(){};
};

class Incrementer : public IterableState
{
private:
  int _value;

public:
  Incrementer() : _value(0){};

  virtual void Iterate()
  {
    std::cout << "Incrementeer Iterate" << ::std::endl;
    _value += 1;
  }
  int Get()
  {
    return _value;
  }
};

class Reporter : public IterableState
{
public:
  virtual void Iterate()
  {
    std::cout << "Reporter Iterate" << ::std::endl;
  }
  void Report(Incrementer incrementer)
  {
    std::cout << incrementer.Get() << std::endl;
  }
};

#endif
