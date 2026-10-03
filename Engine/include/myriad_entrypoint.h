#include "myriad.h"

namespace Myriad
{
    // Clients of the shared library use this to create their application.
    extern Application *CreateApplication();
} // namespace Myriad

int main(int argc, char **argv)
{
    auto app = Myriad::CreateApplication();
    app->Run();
    delete app;
    return 0;
}
