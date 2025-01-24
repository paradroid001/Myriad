#ifndef MYRIAD_ASSET_TEXTUREPROVIDER
#define MYRIAD_ASSET_TEXTUREPROVIDER

#include "asset/AssetProvider.h"
#include "core/core.h"

namespace Myriad
{
    class MYR_API TextureProvider : public AssetProvider
    {
      public:
        // Anything specific to textures?
        virtual void *GetTexPtr() = 0;
    };
} // namespace Myriad

#endif
