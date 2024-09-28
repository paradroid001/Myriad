#include "asset/AssetManager.h"
#include "io/Log.h"

#include "core/memory/Allocator.h"
#include "core/memory/MyrHandle.h"
#include "io/Log.h"
#include <memory> //shared pointer

namespace Myriad
{
    AssetManager::AssetManager() { MYR_CORE_INFO("Asset Manager Constructed"); }
    AssetManager::~AssetManager() { MYR_CORE_INFO("Asset Manager Destructed"); }

    /*
      Get a texture handle.
      If it's not loaded, load it.
    */
    TexHandle_T AssetManager::GetTexture(std::string path)
    {
        TexHandle_T ret = TEXHANDLE_INVALID;
        MapPath2Handle_T::iterator path_search = map_paths_.find(path);
        if (path_search != map_paths_.end())
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
            MyrHandle<Texture2D> h =
                texture_allocator_.Alloc<Texture2D>(&texture_allocator_);
            Texture2D *ptex = h.Get();
            MyrHandle_T handle = h.Handle();
            // load it
            bool success = ptex->Load(path);
            if (!success)
            {
                MYR_CORE_ERROR("Could not load texture {0}", path);
            }
            else
            {
                MYR_CORE_INFO("Loaded texture {0}", path);

                texture_nodes_[textures_allocated_].asset = ptex;
                texture_nodes_[textures_allocated_].count = 1;

                map_textures_[handle] = &texture_nodes_[textures_allocated_];
                map_paths_[path] = handle;
                textures_allocated_ += 1;
                ret = handle;
            }
        }
        return ret;
    }

    Texture2D *AssetManager::GetTexturePointer(TexHandle_T handle)
    {
        return texture_allocator_.At<Texture2D>(handle);
    }

    void AssetManager::ReleaseTexture(TexHandle_T handle)
    {
        MYR_CORE_INFO("Asked to release tex handle {0}", handle);
    }
} // namespace Myriad
