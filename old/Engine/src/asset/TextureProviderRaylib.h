#ifndef MYRIAD_ASSET_TEXTUREPROVIDERRAYLIB_H
#define MYRIAD_ASSET_TEXTUREPROVIDERRAYLIB_H

#include "asset/TextureProvider.h"
#include "core/core.h"
#include "raylib.h"
#include <string>

namespace Myriad
{
    class MYR_API TextureProviderRaylib : public TextureProvider
    {
      private:
        ::Texture2D tex2D_; // the raylib texture.

      public:
        ~TextureProviderRaylib();
        bool Init() override;
        bool Shutdown() override;
        bool Load(const std::string path) override;
        bool Unload() override;
        void *GetTexPtr() override;
    };
} // namespace Myriad

#endif
