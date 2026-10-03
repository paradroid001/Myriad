#include "myriad_types.h"

namespace Myriad::Util
{
    // Make sure static var is defined in a translation unit
    Random *Random::s_rand_instance_ = nullptr;
} // namespace Myriad
