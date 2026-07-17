#include "myriad.h"

namespace Myriad
{
    // Make sure static var is defined in a translation unit
    MyrRandom *MyrRandom::s_rand_instance_ = nullptr;
} // namespace Myriad
