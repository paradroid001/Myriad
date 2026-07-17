#ifndef _MYRIAD_ASSET_MYRFONT_H_
#define _MYRIAD_ASSET_MYRFONT_H_

#include "myriad.h" //Font, AssetProvider

#include "asset/FontProvider.h" //FontProvider

namespace Myriad
{
    class MyrFont : public Asset, public FontProvider
    {
      private:
        FontProvider *font_provider_;

      public:
        MyrFont();
        virtual ~MyrFont();
        virtual bool Init() override;
        virtual bool Shutdown() override;
        virtual AssetID_t Load(const std::string path) override;
        virtual bool Unload(AssetID_t id) override;

        virtual void *GetFontPtr() = 0;
    };
} // namespace Myriad
#endif
