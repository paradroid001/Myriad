#ifndef MYRIAD_ASSET_ASSETPROVIDER_H
#define MYRIAD_ASSET_ASSETPROVIDER_H

#include "asset/MyrAsset.h"
#include "core/Provider.h"
#include "core/core.h"
#include <string>

namespace Myriad
{
    class MYR_API AssetProvider : public Provider, public IMyrAsset
    {
      public:
        bool Init() = 0;
        bool Shutdown() = 0;
        bool Load(const std::string path) = 0;
        bool Unload() = 0;
    };
} // namespace Myriad

#endif
