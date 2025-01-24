#ifndef MYRIAD_ASSET_MYRASSET_H
#define MYRIAD_ASSET_MYRASSET_H

#include "core/core.h"
#include <string>

namespace Myriad
{
    class MYR_API IMyrAsset
    {
      public:
        virtual bool Load(const std::string path) = 0;
        virtual bool Unload() = 0;
    };
} // namespace Myriad
#endif
