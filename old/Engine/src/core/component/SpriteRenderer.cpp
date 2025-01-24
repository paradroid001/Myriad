#include "core/component/SpriteRenderer.h"
#include "asset/AssetManager.h"
#include "core/component/Transform.h"
#include "io/Log.h"
#include "rendering/Renderer.h"
// TODO: until GetComponent<>
#include "core/object/GameObject.h"

namespace Myriad
{
    SpriteRenderer::SpriteRenderer(AssetManager *p_asset_manager,
                                   std::string texture_path)
        : MyrComponentBase(), texture_path_(texture_path),
          p_asset_manager_(p_asset_manager)
    {
    }
    SpriteRenderer::~SpriteRenderer()
    {
        MYR_CORE_TRACE("Sprite Renderer needs to deallocate here..");
        ReleaseComponent();
    }

    bool SpriteRenderer::InitComponent(MyrObject *owner)
    {
        // Init the component
        MyrComponent::InitComponent(owner);
        // Load the texture
        h_tex_ = p_asset_manager_->GetTexture("res/carrot.png");
        // TODO should return false if can't find the tex.
        return true;
    }
    bool SpriteRenderer::ReleaseComponent()
    {
        // TODO should return correctly here in case of problems
        p_asset_manager_->ReleaseTexture(h_tex_);
        return true;
    }

    void SpriteRenderer::Draw(Renderer &renderer)
    {
        GameObject *g = static_cast<GameObject *>(owner_);
        Vector3 p = g->GetTransform().GetPosition();
        renderer.DrawTexture(*p_asset_manager_->GetTexturePointer(h_tex_),
                             {p.x, p.y}, {255, 255, 255, 255});
    }
} // namespace Myriad
