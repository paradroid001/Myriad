#include "asset/FontProviderRaylib.h"
#include "raylib.h"
#include <string>

namespace Myriad
{
    FontProviderRaylib::~FontProviderRaylib() {}
    bool FontProviderRaylib::Init() { return true; }
    bool FontProviderRaylib::Shutdown() { return true; }
    bool FontProviderRaylib::Load(const std::string path)
    {
        font_ = LoadFont(path.c_str());
        return true;
    }
    bool FontProviderRaylib::Unload()
    {
        UnloadFont(font_);
        return true;
    }

    void *FontProviderRaylib::GetFontPtr()
    {
        ::Font *f = &font_;
        return static_cast<void *>(f);
    }
} // namespace Myriad
