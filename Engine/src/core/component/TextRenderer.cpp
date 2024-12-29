#include "core/component/TextRenderer.h"
#include "asset/AssetManager.h"
#include "core/component/Transform.h"
#include "core/object/GameObject.h"
#include "io/Log.h"
#include "rendering/Renderer.h"

namespace Myriad
{
    TextRenderer::TextRenderer(AssetManager *p_asset_manager, std::string text,
                               std::string font_path, float font_size)
        : MyrComponentBase(), p_asset_manager_(p_asset_manager), text_(text),
          font_path_(font_path), font_size_(font_size)
    {
    }

    TextRenderer::~TextRenderer()
    {
        MYR_CORE_TRACE("Text renderer needs to deallocate here");
        ReleaseComponent();
    }

    bool TextRenderer::InitComponent(MyrObject *owner)
    {
        // init the component
        MyrComponent::InitComponent(owner);
        // Load the font
        h_font_ = p_asset_manager_->GetFont(font_path_);
        // TODO should return false if can't load the font
        return true;
    }
    bool TextRenderer::ReleaseComponent()
    {
        // TODO should return correctly from here in case of problems.
        p_asset_manager_->ReleaseFont(h_font_);
        return true;
    }

    void TextRenderer::Draw(Renderer &renderer)
    {
        GameObject *g = static_cast<GameObject *>(owner_);
        Vector3 pos = g->GetTransform().GetPosition();
        int fontsize = 10;
        renderer.DrawText(*p_asset_manager_->GetFontPointer(h_font_), text_,
                          {pos.x, pos.y}, fontsize, {255, 255, 255, 255});
    }

} // namespace Myriad
