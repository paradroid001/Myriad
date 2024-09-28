#ifndef MYRIAD_ASSET_ASSETMANAGER_H
#define MYRIAD_ASSET_ASSETMANAGER_H
#include "core/core.h"
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "asset/Texture2D.h"
#include "core/memory/Allocator.h"
#include "core/memory/MyrHandle.h"

namespace Myriad
{
    const uint16_t MAX_TEXTURES = 500;
    const uint16_t TEXHANDLE_INVALID = MAX_TEXTURES + 1;
    typedef MyrHandle_T TexHandle_T;

    template <class T> struct AssetNode
    {
        T *asset;
        uint16_t count; // how many things have this loaded?
    };

    typedef std::map<TexHandle_T, AssetNode<Texture2D> *> MapHandle2Texture_T;
    typedef std::map<std::string, TexHandle_T> MapPath2Handle_T;
    class AssetManager
    {
      protected:
        Allocator texture_allocator_;
        AssetNode<Texture2D> texture_nodes_[MAX_TEXTURES];
        MapHandle2Texture_T map_textures_;
        MapPath2Handle_T map_paths_;
        uint16_t textures_allocated_ = 0;

      public:
        AssetManager();
        ~AssetManager();
        TexHandle_T GetTexture(std::string path);
        Texture2D *GetTexturePointer(TexHandle_T handle);
        void ReleaseTexture(TexHandle_T);
    };
} // namespace Myriad
#endif
