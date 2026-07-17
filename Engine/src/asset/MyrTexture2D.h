#ifndef _MYRIAD_ASSET_MYRTEXTURE2D_H_
#define _MYRIAD_ASSET_MYRTEXTURE2D_H_

#include "myriad.h" //Texture2D, AssetProvider

#include "asset/Texture2DProvider.h" //Texture2DProvider

namespace Myriad
{
    class MyrTexture2D : public Asset, public Texture2DProvider
    {
      private:
        Texture2DProvider *texture_provider;

      public:
        MyrTexture2D();
        virtual ~MyrTexture2D();
        virtual bool Init() override;
        virtual bool Shutdown() override;
        virtual AssetID_t Load(const std::string path) override;
        virtual bool Unload(AssetID_t id) override;

        virtual void *GetTexPtr() = 0;
    };
} // namespace Myriad
#endif
