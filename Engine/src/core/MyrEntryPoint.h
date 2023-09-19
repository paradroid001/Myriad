#pragma once
#include "MyrApplication.h"

extern Myriad::MyrApplication *Myriad::CreateApplication(); // defined in
                                                            // client.

int main(int argc, char **argv)
{
    printf("Welcome to Myriad Engine\n");
    auto app = Myriad::CreateApplication();
    app->Run();
    delete app;
    return 0;
}