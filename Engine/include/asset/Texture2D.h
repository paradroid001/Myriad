#ifndef MYRIAD_ASSET_TEXTURE2D_H
#define MYRIAD_ASSET_TEXTURE2D_H

#include "asset/TextureProvider.h"
#include "core/core.h"
#include <string>

namespace Myriad
{
  // Tex typedefs
  const MYR_ID_t MAX_TEXTURES = 500;
  const MYR_ID_t TEXHANDLE_INVALID = MAX_TEXTURES + 1;
  typedef MYR_ID_t TexHandle_T;

  class MYR_API Texture2D : public TextureProvider
  {
  private:
    TextureProvider *texture_provider;

  public:
    Texture2D();
    virtual ~Texture2D(); // virtual otherwise compiler complains about allocator delete
    bool Init() override;
    bool Shutdown() override;
    bool Load(const std::string path) override;
    bool Unload() override;
    // A texture provider provides...
    void *GetTexPtr() override;
  };
} // namespace Myriad
#endif
