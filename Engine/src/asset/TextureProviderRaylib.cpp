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
    // If the texture came back with a nonzero
    // texture id, that should have been successful.
    // Note raylib code initialises Texture2D to all zeroes
    // Also http://www.opengl.org/sdk/docs/man/xhtml/glBindTexture.xml
    //   Note about "The value zero is reserved to represent the default texture for each texture target."
    if (tex2D_.id != 0)
      return true;
    return false;
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
