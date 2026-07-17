#ifndef _MYRIAD_ASSET_FONTPROVIDER_H_
#define _MYRIAD_ASSET_FONTPROVIDER_H_

#include "myriad.h" //AssetProvider

#include <memory>

namespace Myriad
{
    class FontProvider : public AssetProvider
    {
      public:
        FontProvider(AssetManager *asset_manager) : AssetProvider(asset_manager)
        {
        }
        virtual ~FontProvider() {}; // todo I hate this.
        virtual bool Init() = 0;
        virtual bool Shutdown() = 0;
        virtual AssetID_t Load(const std::string path) = 0;
        virtual bool Unload(AssetID_t id) = 0;

        virtual void *GetFontPtr(AssetID_t) = 0;
    };
} // namespace Myriad

#endif
