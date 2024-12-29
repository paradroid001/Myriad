#ifndef MYRIAD_ASSET_TEXTURE2D_H
#define MYRIAD_ASSET_TEXTURE2D_H

#include "asset/TextureProvider.h"
#include "core/core.h"
#include "core/memory/Allocator.h"
#include <string>

namespace Myriad
{
    class MYR_API Texture2D : public TextureProvider
    {
      private:
        TextureProvider *texture_provider;

      public:
        Texture2D(Allocator *allocator);
        ~Texture2D();
        bool Init() override;
        bool Shutdown() override;
        bool Load(const std::string path) override;
        bool Unload() override;
        // A texture provider provides...
        void *GetTexPtr() override;
    };
} // namespace Myriad
#endif
