#ifndef _MYRIAD_ASSET_MYRASSETMANAGER_H_
#define _MYRIAD_ASSET_MYRASSETMANAGER_H_

#include "myriad.h" //AssetManager, Texture2D, Font Interfaces

#include "asset/FontProvider.h"
#include "asset/Texture2DProvider.h"

#include "core/MyrContainers.h" //MyrIDSource, MyrIDMap

#include <string>
#include <unordered_map>

namespace Myriad
{

    template <class T> struct AssetNode
    {
        T *asset;
        uint16_t count; // how many things have this loaded?
    };

    // Just to avoid the type errors below, all of this code
    // should go.
    /*
    typedef AssetID_t TexHandle_t;
    typedef AssetID_t FontHandle_t;

    typedef std::unordered_map<TexHandle_t, AssetNode<Texture2D> *>
        MapHandle2Texture_T;
    typedef std::unordered_map<std::string, TexHandle_t> MapPath2TexHandle_T;

    typedef std::unordered_map<FontHandle_t, AssetNode<Font> *>
        MapHandle2Font_T;
    typedef std::unordered_map<std::string, FontHandle_t> MapPath2FontHandle_T;
    */

    class MyrAssetManager : public AssetManager
    {
      private:
        // Allocator<Texture2D, TypeAllocatorDynamic<Texture2D>>
        //     texture_allocator_;
        /*
        AssetNode<Texture2D> texture_nodes_[MAX_TEXTURES];
        MapHandle2Texture_T map_textures_;
        MapPath2TexHandle_T map_tex_paths_;
        uint16_t textures_allocated_ = 0;

        // Allocator<Font, TypeAllocatorDynamic<Font>> font_allocator_;
        AssetNode<Font> font_nodes_[MAX_FONTS];
        MapHandle2Font_T map_fonts_;
        MapPath2FontHandle_T map_font_paths_;
        uint16_t fonts_allocated_ = 0;
        */

        // Two maps for fast lookups
        std::unordered_map<std::string, AssetID_t> map_path_to_id_;
        // I kind of want this to be indices into arrays,
        // but I won't know which array. This is simpler.
        // I'm concerned about the lookup time, I want it to
        // be as small as possibe when we pass to the renderer.
        std::unordered_map<AssetID_t, Asset *> map_id_to_asset_;

        // The arrays live here. Actually no, they should live in the
        // texture and font providers.
        Texture2D textures_[MAX_TEXTURES];
        Font fonts_[MAX_FONTS];

        MyrIDSource id_gen_; // a source for generating unique ids for assets
        std::shared_ptr<FontProvider> font_provider_;
        std::shared_ptr<Texture2DProvider> texture2d_provider_;

      public:
        MyrAssetManager();
        ~MyrAssetManager();

        AssetID_t GenerateID() override
        {
            return static_cast<AssetID_t>(id_gen_.GetNextID());
        }

        Texture2D GetTexture(std::string path) override;
        Asset *GetFont(std::string path) override;

        virtual std::shared_ptr<AssetProvider> GetTextureProvider() override
        {
            return texture2d_provider_;
        }
        virtual std::shared_ptr<AssetProvider> GetFontProvider() override
        {
            return font_provider_;
        }

        // virtual TexHandle_T GetTexture(std::string path) override;
        // virtual void *GetTexturePointer(MYR_ID_t handle) override;
        // virtual void ReleaseTexture(TexHandle_T handle) override;

        // virtual FontHandle_T GetFont(std::string path) override;
        // virtual void *GetFontPointer(MYR_ID_t handle) override;
        // virtual void ReleaseFont(FontHandle_T handle) override;
    };
} // namespace Myriad

#endif
