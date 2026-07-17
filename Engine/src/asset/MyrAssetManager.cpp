#include "myriad.h"

#include "asset/MyrAssetManager.h"
#include "io/Logging.h"

#include <memory> //shared pointer

// Because this is a raylib centric lib
#include "asset/FontProviderRaylib.h"
#include "asset/Texture2DProviderRaylib.h"

namespace Myriad
{
    MyrAssetManager::MyrAssetManager()
    {
        MYR_CORE_INFO("MyrAsset Manager Constructed");

        map_path_to_id_.clear();
        map_id_to_asset_.clear();

        font_provider_ = std::make_shared<FontProviderRaylib>(this);
        texture2d_provider_ = std::make_shared<Texture2DProviderRaylib>(this);
    }
    MyrAssetManager::~MyrAssetManager()
    {
        MYR_CORE_INFO("MyrAsset Manager Destructing");
        font_provider_->Shutdown();
        texture2d_provider_->Shutdown();
        int count = 0;
        for (auto it : map_id_to_asset_)
        {
            MYR_CORE_TRACE("Deleting Asset %d: %x", it.first, it.second);
            // free each asset
            // TODO: Actually the asset is freed in the TextureProviderRaylib
            //       It wasn't 'newed' - so when erase was called
            //                           it ceased to be.
            // delete it.second;
            count += 1;
        }
        MYR_CORE_INFO("MyrAsset Manager Destructed, %d assets freed.", count);
    }

    Texture2D MyrAssetManager::GetTexture(std::string path)
    {
        // Implementation for getting a texture
        // TODO nothing here yet for caching against path
        AssetID_t id = texture2d_provider_->Load(path);
        if (id != MYRIAD_INVALID_ID)
        {
            void *tex_ptr = texture2d_provider_->GetTexPtr(id);
            if (tex_ptr == nullptr)
            {
                MYR_CORE_ERROR(
                    "Texture provider returned invalid pointer for texture {0}",
                    path.c_str());
            }
            else
            {
                // Make Texture2D instead of Asset.
                // I don't think I want to do this in maps.
                Texture2D *tex = texture2d_provider_->GetTexAsset(id);
                map_id_to_asset_[id] = tex;
                map_path_to_id_[path] = id;
                return *tex;
            }
        }
        else
        {
            MYR_CORE_ERROR("Failed to load texture %s", path.c_str());
        }
        // Return an invalid texture
        return Texture2D(MYRIAD_INVALID_ID, path, nullptr);
    }

    Asset *MyrAssetManager::GetFont(std::string path)
    {
        // Implementation for getting a font
        // TODO nothing here yet for caching against path
        AssetID_t id = font_provider_->Load(path);
        if (id != MYRIAD_INVALID_ID)
        {
            void *font_ptr = font_provider_->GetFontPtr(id);
            if (font_ptr == nullptr)
            {
                MYR_CORE_ERROR(
                    "Font provider returned invalid pointer for font {0}",
                    path.c_str());
                return nullptr;
            }
            map_id_to_asset_[id] = new Asset(id, path, font_ptr);
            map_path_to_id_[path] = id;
            return map_id_to_asset_[id];
        }
        else
        {
            MYR_CORE_ERROR("Failed to load font %s", path.c_str());
            return nullptr;
        }
    }

    /*
      Get a texture handle.
      If it's not loaded, load it.
    */
    /*
    TexHandle_t MyrAssetManager::GetTexture(std::string path)
    {
        TexHandle_t ret = TEXHANDLE_INVALID;
        MapPath2TexHandle_T::iterator path_search = map_tex_paths_.find(path);
        if (path_search != map_tex_paths_.end())
        {
            MapHandle2Texture_T::iterator handle_search =
                map_textures_.find(path_search->second);
            if (handle_search != map_textures_.end())
            {
                // MYR_CORE_INFO("Texture {0} already loaded.", path);
                ret = handle_search->first;
            }
            else
            {
                // error
                MYR_CORE_ERROR("Loaded texture path found with no asset");
            }
        }
        else
        {
            // it didn't exist.
            // Create it
            MYR_ID_t tex_id = texture_allocator_.Alloc<Texture2D>();
            Texture2D *p_tex = texture_allocator_.Get(tex_id);
            // load it
            bool success = p_tex->Load(path);
            if (!success)
            {
                MYR_CORE_ERROR("Could not load texture {0}", path);
            }
            else
            {
                MYR_CORE_INFO("Loaded texture {0}", path);

                texture_nodes_[textures_allocated_].asset = p_tex;
                texture_nodes_[textures_allocated_].count = 1;

                map_textures_[tex_id] = &texture_nodes_[textures_allocated_];
                map_tex_paths_[path] = tex_id;
                textures_allocated_ += 1;
                ret = tex_id;
            }
        }
        return ret;
    }

    void *MyrAssetManager::GetTexturePointer(MYR_ID_t handle)
    {
        return texture2d_provider_->GetTexPtr(handle);
    }

    void MyrAssetManager::ReleaseTexture(TexHandle_t handle)
    {
        MYR_CORE_INFO("Asked to release tex handle {0} (did nothing)", handle);
    }

    FontHandle_t MyrAssetManager::GetFont(std::string path)
    {
        FontHandle_t ret = FONTHANDLE_INVALID;
        MapPath2FontHandle_T::iterator path_search = map_font_paths_.find(path);
        if (path_search != map_font_paths_.end())
        {
            MapHandle2Font_T::iterator handle_search =
                map_fonts_.find(path_search->second);
            if (handle_search != map_fonts_.end())
            {
                ret = handle_search->first;
            }
            else
            {
                MYR_CORE_ERROR("Loaded font path found with no asset");
            }
        }
        else
        {
            // it didn't exist
            // Create it
            MYR_ID_t font_id = font_allocator_.Alloc<Font>();
            Font *pfont = font_allocator_.Get(font_id);
            // Load it
            bool success = pfont->Load(path);
            if (!success)
            {
                MYR_CORE_ERROR("Could not load font {0}", path);
            }
            else
            {
                MYR_CORE_INFO("Loaded font {0}", path);
                {
                    font_nodes_[fonts_allocated_].asset = pfont;
                    font_nodes_[fonts_allocated_].count = 1;

                    map_fonts_[font_id] = &font_nodes_[fonts_allocated_];
                    map_font_paths_[path] = font_id;
                    fonts_allocated_ += 1;
                    ret = font_id;
                }
            }
        }
        return ret;
    }

    void *MyrAssetManager::GetFontPointer(MYR_ID_t handle)
    {
        return font_provider_->GetFontPtr(handle);
    }

    void MyrAssetManager::ReleaseFont(MYR_ID_t handle)
    {
        MYR_CORE_INFO("Asked to release font handle {0}, did nothing", handle);
    }
    */
} // namespace Myriad
