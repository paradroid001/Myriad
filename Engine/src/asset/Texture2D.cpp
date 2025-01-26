#include "asset/Texture2D.h"
#include "core/MyrAlloc.h"

#ifdef MYRIAD_INTERNAL
#include "core/config.h"
#else

#endif

#if MYRIAD_RENDERER == RENDERER_RAYLIB
#include "asset/TextureProviderRaylib.h"
#endif

namespace Myriad
{
  Texture2D::Texture2D() : TextureProvider()
  {
    texture_provider = nullptr;
#if MYRIAD_RENDERER == RENDERER_RAYLIB
    texture_provider = new TextureProviderRaylib();
#endif
  }

  Texture2D::~Texture2D()
  {
    // This is causing a segfault
    // if (texture_provider != nullptr)
    //     delete texture_provider;
    // TODO once this is fixed, look at the other asset providers.
  }

  bool Texture2D::Init() { return texture_provider->Init(); }
  bool Texture2D::Shutdown() { return texture_provider->Shutdown(); }
  bool Texture2D::Load(const std::string path)
  {
    return texture_provider->Load(path);
  }
  bool Texture2D::Unload() { return texture_provider->Unload(); }
  void *Texture2D::GetTexPtr() { return texture_provider->GetTexPtr(); }

} // namespace Myriad
