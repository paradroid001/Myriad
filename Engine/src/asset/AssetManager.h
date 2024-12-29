#ifndef MYRIAD_ASSET_ASSETMANAGER_H
#define MYRIAD_ASSET_ASSETMANAGER_H
#include "core/core.h"
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "asset/Font.h"
#include "asset/Texture2D.h"
#include "core/memory/Allocator.h"
#include "core/memory/MyrHandle.h"

namespace Myriad
{

    template <class T> struct AssetNode
    {
        T *asset;
        uint16_t count; // how many things have this loaded?
    };

    // Tex typedefs
    const uint16_t MAX_TEXTURES = 500;
    const uint16_t TEXHANDLE_INVALID = MAX_TEXTURES + 1;
    typedef MyrHandle_T TexHandle_T;
    typedef std::map<TexHandle_T, AssetNode<Texture2D> *> MapHandle2Texture_T;
    typedef std::map<std::string, TexHandle_T> MapPath2TexHandle_T;

    // Font typedefs
    const uint16_t MAX_FONTS = 32;
    const uint16_t FONTHANDLE_INVALID = MAX_FONTS + 1;
    typedef MyrHandle_T FontHandle_T;
    typedef std::map<FontHandle_T, AssetNode<Font> *> MapHandle2Font_T;
    typedef std::map<std::string, FontHandle_T> MapPath2FontHandle_T;

    class AssetManager
    {
      protected:
        Allocator texture_allocator_;
        AssetNode<Texture2D> texture_nodes_[MAX_TEXTURES];
        MapHandle2Texture_T map_textures_;
        MapPath2TexHandle_T map_tex_paths_;
        uint16_t textures_allocated_ = 0;

        Allocator font_allocator_;
        AssetNode<Font> font_nodes_[MAX_FONTS];
        MapHandle2Font_T map_fonts_;
        MapPath2FontHandle_T map_font_paths_;
        uint16_t fonts_allocated_ = 0;

      public:
        AssetManager();
        ~AssetManager();
        TexHandle_T GetTexture(std::string path);
        Texture2D *GetTexturePointer(TexHandle_T handle);
        void ReleaseTexture(TexHandle_T);

        FontHandle_T GetFont(std::string path);
        Font *GetFontPointer(FontHandle_T handle);
        void ReleaseFont(FontHandle_T);
    };
} // namespace Myriad
#endif
