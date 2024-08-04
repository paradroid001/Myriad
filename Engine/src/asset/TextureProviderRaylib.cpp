#include "asset/TextureProviderRaylib.h"
#include "raylib.h"
#include <string>

namespace Myriad
{
    TextureProviderRaylib::~TextureProviderRaylib() {}
    bool TextureProviderRaylib::Init() { return true; }
    bool TextureProviderRaylib::Shutdown() { return true; }
    bool TextureProviderRaylib::Load(const std::string path)
    {
        tex2D_ = LoadTexture(path.c_str());
        return true;
    }
    bool TextureProviderRaylib::Unload()
    {
        UnloadTexture(tex2D_);
        return true;
    }

    void *TextureProviderRaylib::GetTexPtr()
    {
        ::Texture2D *t = &tex2D_;
        return static_cast<void *>(t);
    }
} // namespace Myriad
