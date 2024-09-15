#ifndef MYRIAD_ASSET_ASSETMANAGER_H
#define MYRIAD_ASSET_ASSETMANAGER_H
#include "core/core.h"
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "asset/TextureProvider.h"

namespace Myriad
{
    template <class T> struct AssetNode
    {
        std::unique_ptr<T> asset;
        uint16_t count;
    };

    const uint16_t MAX_TEXTURES = 500;
    const uint16_t TEXHANDLE_INVALID = MAX_TEXTURES + 1;
    typedef uint16_t TexHandle_T;
    typedef std::map<std::string, AssetNode<TextureProvider> *> MapTexture_T;
    class AssetManager
    {
      protected:
        MapTexture_T map_textures_;

      public:
        AssetManager();
        ~AssetManager();
        TexHandle_T GetTexture(std::string path);
        void ReleaseTexture(TexHandle_T);
    };
} // namespace Myriad
#endif
