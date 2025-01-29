#include "asset/AssetManager.h"
#include "io/MyrLogging.h"
#include <memory> //shared pointer

namespace Myriad
{
  AssetManager::AssetManager()
  {
    MYR_CORE_INFO("Asset Manager Constructed");
    texture_allocator_.Init(MAX_TEXTURES);
    font_allocator_.Init(MAX_FONTS);
    map_font_paths_.clear();
    map_fonts_.clear();
  }
  AssetManager::~AssetManager() { MYR_CORE_INFO("Asset Manager Destructed"); }

  /*
    Get a texture handle.
    If it's not loaded, load it.
  */
  TexHandle_T AssetManager::GetTexture(std::string path)
  {
    TexHandle_T ret = TEXHANDLE_INVALID;
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

  Texture2D *AssetManager::GetTexturePointer(TexHandle_T handle)
  {
    return texture_allocator_.Get(handle);
  }

  void AssetManager::ReleaseTexture(TexHandle_T handle)
  {
    MYR_CORE_INFO("Asked to release tex handle {0} (did nothing)", handle);
  }

  FontHandle_T AssetManager::GetFont(std::string path)
  {
    FontHandle_T ret = FONTHANDLE_INVALID;
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

  Font *AssetManager::GetFontPointer(FontHandle_T handle)
  {
    return font_allocator_.Get(handle);
  }

  void AssetManager::ReleaseFont(FontHandle_T handle)
  {
    MYR_CORE_INFO("Asked to release font handle {0}, did nothing", handle);
  }
} // namespace Myriad
