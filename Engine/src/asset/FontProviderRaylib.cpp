#include "asset/FontProviderRaylib.h"

#include "io/Logging.h"

#include "raylib.h"

#include <string>

namespace Myriad
{
    FontProviderRaylib::FontProviderRaylib(AssetManager *asset_manager)
        : FontProvider(asset_manager)
    {
    }

    FontProviderRaylib::~FontProviderRaylib() {}

    bool FontProviderRaylib::Init() { return true; }

    bool FontProviderRaylib::Shutdown()
    {
        MYR_CORE_INFO("Font Provider Raylib Shutting down.");
        int count = 0;
        while (!fonts_.empty())
        {
            auto it = fonts_.begin();
            // Unload by id
            Unload(it->first);
            count += 1;
        }
        fonts_.clear();
        MYR_CORE_INFO(
            "Font Provider Raylib shutdown complete, %d fonts unloaded", count);
        return true;
    }

    AssetID_t FontProviderRaylib::Load(const std::string path)
    {
        AssetID_t newid = MYRIAD_INVALID_ID;
        ::Font font_ = LoadFont(path.c_str());
        if (font_.texture.id == 0)
        {
            MYR_CORE_ERROR("Failed to load font {0}", path.c_str());
        }
        else
        {
            MYR_CORE_INFO("Loaded font {0}", path.c_str());
            newid = asset_manager_->GenerateID();
            fonts_[newid] = font_;
        }
        return newid;
    }
    bool FontProviderRaylib::Unload(AssetID_t id)
    {
        if (fonts_.count(id) == 1)
        {
            ::Font font = fonts_[id];
            ::UnloadFont(font);
            MYR_CORE_TRACE("Unloaded font %d", id);
            fonts_.erase(id);
            return true;
        }
        // The font didn't exist
        return false;
    }

    void *FontProviderRaylib::GetFontPtr(AssetID_t id)
    {
        if (fonts_.count(id) == 1)
        {
            ::Font *f = &fonts_[id];
            return static_cast<void *>(f);
        }
        MYR_CORE_ERROR("Font with id {0} not found", id);
        return nullptr;
    }
} // namespace Myriad
