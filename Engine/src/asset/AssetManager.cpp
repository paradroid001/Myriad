#include "asset/AssetManager.h"
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
        MapTexture_T::iterator search = map_textures_.find(path);
        if (search != map_textures_.end())
        {
            AssetNode<TextureProvider> *node = search->second;
            node->count += 1;
            // TODO this is an 'index' I guess
            return search - map_textures_.begin();
        }
    }
} // namespace Myriad
