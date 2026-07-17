#ifndef MYRIAD_ASSET_FONTPROVIDERRAYLIB_H
#define MYRIAD_ASSET_FONTPROVIDERRAYLIB_H

#include "myriad.h"

#include "asset/FontProvider.h"
#include "raylib.h"
#include <string>
#include <unordered_map>

namespace Myriad
{
    class FontProviderRaylib : public FontProvider
    {
      protected:
        std::unordered_map<AssetID_t, ::Font> fonts_;

      public:
        FontProviderRaylib(AssetManager *asset_manager);
        ~FontProviderRaylib();
        bool Init() override;
        bool Shutdown() override;
        AssetID_t Load(const std::string path) override;
        bool Unload(AssetID_t id) override;

        void *GetFontPtr(AssetID_t id) override;
    };
} // namespace Myriad

#endif
