#include "asset/Font.h"
#include "core/memory/Allocator.h"

#ifdef MYRIAD_INTERNAL
    #include "core/MyriadConfig.h"
#else
#endif

#if MYRIAD_RENDERER == RAYLIB
    #include "asset/FontProviderRaylib.h"
#endif

namespace Myriad
{
    Font::Font(Allocator *allocator) : FontProvider()
    {
        font_provider_ = nullptr;
        // #if MYRIAD_RENDERER == RAYLIB
        font_provider_ = new FontProviderRaylib();
        // #endif
    }

    Font::~Font()
    {
        // don't do anything for now: see the note in Texture2D.cpp
    }

    bool Font::Init() { return font_provider_->Init(); }
    bool Font::Shutdown() { return font_provider_->Shutdown(); }
    bool Font::Load(const std::string path)
    {
        return font_provider_->Load(path);
    }
    bool Font::Unload() { return font_provider_->Unload(); }

    void *Font::GetFontPtr() { return font_provider_->GetFontPtr(); }
} // namespace Myriad
