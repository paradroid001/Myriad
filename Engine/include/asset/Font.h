#ifndef MYRIAD_ASSET_FONT_H
#define MYRIAD_ASSET_FONT_H

#include "core/core.h"
#include "asset/FontProvider.h"

#include <string>

namespace Myriad
{
  // Font typedefs
  const MYR_ID_t MAX_FONTS = 32;
  const MYR_ID_t FONTHANDLE_INVALID = MAX_FONTS + 1;
  typedef MYR_ID_t FontHandle_T;

  class MYR_API Font : public FontProvider
  {
  private:
    FontProvider *font_provider_;

  public:
    Font();
    virtual ~Font(); // virtual otherwise compiler complains about allocator delete
    bool Init() override;
    bool Shutdown() override;
    bool Load(const std::string path) override;
    bool Unload() override;
    // A font provider provides...
    void *GetFontPtr() override;
  };
} // namespace Myriad

#endif
