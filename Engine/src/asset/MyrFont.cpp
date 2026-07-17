#include "myriad.h"

#include "asset/MyrFont.h"
// TODO: only if raylib
#include "asset/FontProviderRaylib.h"

namespace Myriad
{
    MyrFont::MyrFont() : FontProvider()
    {
        // TODO only if raylib
        font_provider_ = new FontProviderRaylib();
    }

    MyrFont::~MyrFont()
    {
        // don't do anything for now: see the note in Texture2D.cpp
    }

    bool MyrFont::Init() { return font_provider_->Init(); }
    bool MyrFont::Shutdown() { return font_provider_->Shutdown(); }
    bool MyrFont::Load(const std::string path)
    {
        return font_provider_->Load(path);
    }
    bool MyrFont::Unload() { return font_provider_->Unload(); }

    void *MyrFont::GetFontPtr() { return font_provider_->GetFontPtr(); }
} // namespace Myriad
