#ifndef MYRIAD_ASSET_FONTPROVIDER_H
#define MYRIAD_ASSET_FONTPROVIDER_H

#include "asset/AssetProvider.h"
#include "core/core.h"

namespace Myriad
{
    class MYR_API FontProvider : public AssetProvider
    {
      public:
        virtual void *GetFontPtr() = 0;
    };
} // namespace Myriad

#endif
