#include "main.h"
#include <myriad.h>

Sample::Sample() {}
Sample::~Sample() {}

void Sample::Run()
{
    while (true)
    {
    }
}

Myriad::MyrApplication *Myriad::CreateApplication()
{
    // implementation of CreateApplication
    return new Sample();
}