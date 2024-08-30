#include "core/component/SpriteRenderer.h"
#include "core/component/Transform.h"
#include "core/memory/Allocator.h"
#include "io/Log.h"
#include "rendering/Renderer.h"
// TODO: until GetComponent<>
#include "core/object/GameObject.h"

namespace Myriad
{
    SpriteRenderer::SpriteRenderer(Allocator *p_allocator,
                                   std::string texture_path)
        : MyrComponent(), texture_path_(texture_path), p_allocator_(p_allocator)
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
        p_tex_ = new Myriad::Texture2D(p_allocator_);
        p_tex_->Load(texture_path_);
        // TODO should return false if can't find the tex.
        return true;
    }
    bool SpriteRenderer::ReleaseComponent()
    {
        // TODO should return correctly here in case of problems
        p_tex_->Unload();
        delete p_tex_;
        return true;
    }

    void SpriteRenderer::Draw(Renderer &renderer)
    {
        GameObject *g = static_cast<GameObject *>(owner_);
        Vector3 p = g->GetTransform().GetPosition();
        renderer.DrawTexture(*p_tex_, {p.x, p.y}, {255, 255, 255, 255});
    }
} // namespace Myriad
