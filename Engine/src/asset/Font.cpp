#include "core/MyrAlloc.h"
#include "asset/Font.h"

#ifdef MYRIAD_INTERNAL
#include "core/config.h"
#else
#endif

#if MYRIAD_RENDERER == RENDERER_RAYLIB
#include "asset/FontProviderRaylib.h"
#endif

namespace Myriad
{
  Font::Font() : FontProvider()
  {
    font_provider_ = nullptr;
#if MYRIAD_RENDERER == RENDERER_RAYLIB
    font_provider_ = new FontProviderRaylib();
#endif
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
