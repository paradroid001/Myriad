#ifndef MYRIAD_CORE_MYRENTRYPOINT_H
#define MYRIAD_CORE_MYRENTRYPOINT_H

#include "myriad.h" //for MyrApplication

extern Myriad::MyrApplication *Myriad::CreateApplication();

#ifdef MYRIAD_HOSTED_ENTRYPOINT
extern "C" Myriad::MyrGameApplication *Myriad_CreateHostedGame()
{
    return static_cast<Myriad::MyrGameApplication *>(
        Myriad::CreateApplication());
}

extern "C" void
Myriad_DestroyHostedGame(Myriad::MyrGameApplication *application)
{
    delete application;
}
#else
int main(int argc, char **argv)
{
    auto app = Myriad::CreateApplication();
    app->Run();
    delete app;
    return 0;
}
#endif
#endif
