#include "myriad.h"

#include "asset/MyrTexture2D.h"
// TODO only include this if the engine config is raylib
#include "asset/Texture2DProviderRaylib.h"

namespace Myriad
{
    MyrTexture2D::MyrTexture2D() : Texture2DProvider()
    {
        texture_provider = nullptr;
        // TODO: if the engine config is raylib, then instance a raylib one.
        texture_provider = new Texture2DProviderRaylib();
    }

    MyrTexture2D::~MyrTexture2D()
    {
        // This is causing a segfault
        // if (texture_provider != nullptr)
        //     delete texture_provider;
        // TODO once this is fixed, look at the other asset providers.
    }

    bool MyrTexture2D::Init() { return texture_provider->Init(); }
    bool MyrTexture2D::Shutdown() { return texture_provider->Shutdown(); }
    AssetID_t MyrTexture2D::Load(const std::string path)
    {
        return texture_provider->Load(path);
    }
    bool MyrTexture2D::Unload(AssetID_t id)
    {
        return texture_provider->Unload();
    }
    void *MyrTexture2D::GetTexPtr() { return texture_provider->GetTexPtr(); }
} // namespace Myriad
