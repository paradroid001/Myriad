#include <iostream>
#include "Allocator.h"
#include "Handle.h"

int main(int argc, char **argv)
{

  Allocator *alloc = new Allocator();
  Handle<std::string> *h_s = alloc->Make<std::string>("Hi World");
  Handle<int> *h_i = alloc->Make<int>(7);

  std::cout << *(h_s->Get()) << ", I have " << *(h_i->Get()) << " bananas." << std::endl;

  Handle<std::string> *h2_s = alloc->Alloc<std::string>("Bye World");
  Handle<int> *h2_i = alloc->Alloc<int>(9);

  std::cout << *(h2_s->Get()) << ", I have " << *(h2_i->Get()) << " grapes." << std::endl;

  return 0;
}
