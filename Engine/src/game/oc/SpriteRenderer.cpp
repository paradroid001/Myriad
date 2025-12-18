#include "game/oc/SpriteRenderer.h"
#include "game/oc/Transform.h"
#include "game/oc/GameObject.h"
#include "io/MyrLogging.h"
#include "core/MyrProfiling.h"

namespace Myriad::ObjectComponent
{
  using namespace Myriad;

  // SpriteRenderer::SpriteRenderer(TexHandle_T texid)
  //{
  //   texid_ = texid;
  //   alloced_ = false;
  // }
  void SpriteRenderer::Init(AssetManager *asset_manager, std::string &path)
  {
    p_asset_manager_ = asset_manager;
    texid_ = p_asset_manager_->GetTexture(path);
    alloced_ = true;
    SetUpdateable(true);
    SetRenderable(true);
  }
  SpriteRenderer::~SpriteRenderer()
  {
    if (alloced_)
    {
      MYR_CORE_ERROR("Sprite Renderer Component needs the asset manager to unload the texture");
      // something like
      //  asset_manager.ReleaseTexture(texid_);
    }
  }

  void SpriteRenderer::Update(float dt)
  {
  }

  void SpriteRenderer::Render(Renderer &renderer)
  {
    // we need the transform to draw.
    Transform *p_transform = GetOwner()->GetComponent<Transform>();
    renderer.DrawTexture(texid_,
                         {p_transform->GetPosition().x, p_transform->GetPosition().y},
                         {255, 255, 255, 255});
  }

}
