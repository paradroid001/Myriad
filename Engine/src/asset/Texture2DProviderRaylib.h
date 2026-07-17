#ifndef MYRIAD_ASSET_TEXTUREPROVIDERRAYLIB_H
#define MYRIAD_ASSET_TEXTUREPROVIDERRAYLIB_H

#include "myriad.h" //TextureProvider
#include <string>
#include <unordered_map>

#include "asset/Texture2DProvider.h" //Texture2DProvider
#include "core/MyrContainers.h"

#include "raylib.h"

namespace Myriad
{
    class Texture2DProviderRaylib : public Texture2DProvider
    {
      private:
        std::unordered_map<AssetID_t, Texture2D> texture_assets_;
        // The raylib textures.
        std::unordered_map<AssetID_t, ::Texture2D> textures_;

      public:
        Texture2DProviderRaylib(AssetManager *asset_manager);
        ~Texture2DProviderRaylib();

        // Texture2DProvider overrides
        bool Init() override;
        bool Shutdown() override;
        AssetID_t Load(const std::string path) override;
        bool Unload(AssetID_t id) override;

        virtual Texture2D *GetTexAsset(AssetID_t id) override;
        void *GetTexPtr(AssetID_t id) override;
    };
} // namespace Myriad

#endif
