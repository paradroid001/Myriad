#include <string>

#include "asset/Texture2DProviderRaylib.h"
#include "myriad.h"

#include "io/Logging.h"

#include "raylib.h"

namespace Myriad
{
    Texture2DProviderRaylib::Texture2DProviderRaylib(
        AssetManager *asset_manager)
        : Texture2DProvider(asset_manager)
    {
        // Help stop map churn at the expense of
        // memory.
        texture_assets_.reserve(MAX_TEXTURES);
        textures_.reserve(MAX_TEXTURES);
    }

    Texture2DProviderRaylib::~Texture2DProviderRaylib()
    {
        MYR_CORE_TRACE("Texture2D Provider Raylib destroyed");
    }
    bool Texture2DProviderRaylib::Init() { return true; }
    bool Texture2DProviderRaylib::Shutdown()
    {
        MYR_CORE_INFO("Texture2D Provider Raylib shutdown started");
        int count = 0;
        // Unload all textures
        while (!textures_.empty())
        {
            auto it = textures_.begin();
            // Unload by id
            Unload(it->first);
            count += 1;
        }
        textures_.clear();
        MYR_CORE_INFO(
            "Texture2D Provider Raylib shutdown, %d textures unloaded", count);

        return true;
    }
    AssetID_t Texture2DProviderRaylib::Load(const std::string path)
    {
        // If the load call made it this far, then the
        // texture load is actually needed, i.e. no
        // resource was found for that ID.

        // So first, try to load the texture
        ::Texture2D tex2D = LoadTexture(path.c_str());
        // If the texture came back with a nonzero
        // texture id, that should have been successful.
        // Note raylib code initialises Texture2D to all zeroes
        // Also http://www.opengl.org/sdk/docs/man/xhtml/glBindTexture.xml
        //   Note about "The value zero is reserved to represent the default
        //   texture for each texture target."
        if (tex2D.id != 0)
        {
            // Now get a new id
            AssetID_t newid = asset_manager_->GenerateID();
            textures_[newid] = tex2D;
            texture_assets_[newid] =
                Texture2D(newid, path, static_cast<void *>(&textures_[newid]));
            Texture2D *t = &texture_assets_[newid];

            t->texture_dimensions = {tex2D.width, tex2D.height};
            t->pixel_format = tex2D.format;

            return newid;
        }
        return MYRIAD_INVALID_ID;
    }
    bool Texture2DProviderRaylib::Unload(AssetID_t id)
    {
        // If it exists
        if (textures_.count(id) == 1)
        {
            ::Texture2D tex2D = textures_[id];
            ::UnloadTexture(tex2D);
            MYR_CORE_TRACE("Unloaded texture %d", id);
            textures_.erase(id);
            // TODO: this will remove the map entry,
            // But it doesn't invalidate assets that
            // are out there with this id.
            // Is that something we need to worry about?
            texture_assets_.erase(id);
            return true;
        }
        // If we got here, the id didn't exist
        return false;
    }

    Texture2D *Texture2DProviderRaylib::GetTexAsset(AssetID_t id)
    {
        if (textures_.count(id) == 1)
        {
            return &texture_assets_[id];
        }
        return nullptr;
    }

    void *Texture2DProviderRaylib::GetTexPtr(AssetID_t id)
    {
        if (textures_.count(id) == 1)
        {
            return static_cast<void *>(&textures_[id]);
        }
        return nullptr;
    }
} // namespace Myriad
