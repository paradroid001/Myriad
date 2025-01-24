#ifndef MYRIAD_CORE_COMPONENT_TEXTRENDERER_H
#define MYRIAD_CORE_COMPONENT_TEXTRENDERER_H

#include "asset/AssetManager.h"
#include "core/IDrawable.h"
#include "core/component/MyrComponent.h"
#include "core/core.h"
#include "rendering/Renderer.h"
#include <string>

namespace Myriad
{
    class MYR_API TextRenderer : public MyrComponentBase<TextRenderer>,
                                 public IDrawable
    {
      protected:
        AssetManager *p_asset_manager_;
        std::string text_;
        std::string font_path_;
        float font_size_;
        FontHandle_T h_font_;

      public:
        TextRenderer(AssetManager *p_asset_manager, std::string text,
                     std::string font_path, float font_size);
        virtual ~TextRenderer();

        void SetText(std::string newtext);

        // MyrComponent
        virtual bool InitComponent(MyrObject *owner) override;
        virtual bool ReleaseComponent() override;

        // IDrawable
        virtual void Draw(Renderer &renderer) override;
    };
} // namespace Myriad

#endif
