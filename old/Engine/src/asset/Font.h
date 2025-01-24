#ifndef MYRIAD_ASSET_FONT_H
#define MYRIAD_ASSET_FONT_H

#include "asset/FontProvider.h"
#include "core/core.h"
#include "core/memory/Allocator.h"
#include <string>

namespace Myriad
{
    class MYR_API Font : public FontProvider
    {
      private:
        FontProvider *font_provider_;

      public:
        Font(Allocator *allocator);
        ~Font();
        bool Init() override;
        bool Shutdown() override;
        bool Load(const std::string path) override;
        bool Unload() override;
        // A font provider provides...
        void *GetFontPtr() override;
    };
} // namespace Myriad

#endif
