#ifndef MYRIAD_ASSET_FONTPROVIDERRAYLIB_H
#define MYRIAD_ASSET_FONTPROVIDERRAYLIB_H

#include "asset/FontProvider.h"
#include "core/core.h"
#include "raylib.h"
#include <string>

namespace Myriad
{
    class MYR_API FontProviderRaylib : public FontProvider
    {
      private:
        ::Font font_; // the raylib font
      public:
        ~FontProviderRaylib();
        bool Init() override;
        bool Shutdown() override;
        bool Load(const std::string path) override;
        bool Unload() override;
        void *GetFontPtr() override;
    };
} // namespace Myriad

#endif
