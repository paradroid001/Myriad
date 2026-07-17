#ifndef _MYRIAD_ASSET_TEXTURE2DPROVIDER_H_
#define _MYRIAD_ASSET_TEXTURE2DPROVIDER_H_

#include "myriad.h" //TextureProvider

namespace Myriad
{
    class Texture2DProvider : public AssetProvider
    {
      protected:
      public:
        Texture2DProvider(AssetManager *asset_manager)
            : AssetProvider(asset_manager) {};
        virtual ~Texture2DProvider() {}; // TODO - why do we need to do this?
        virtual bool Init() = 0;
        virtual bool Shutdown() = 0;
        virtual AssetID_t Load(const std::string path) = 0;
        virtual bool Unload(AssetID_t id) = 0;

        // Get the texture asset
        virtual Texture2D *GetTexAsset(AssetID_t id) = 0;
        // Get the underlying raw raylib pointer for rendering.
        virtual void *GetTexPtr(AssetID_t id) = 0;
    };
} // namespace Myriad

#endif
