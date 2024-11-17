#ifndef MYRIAD_CORE_COMPONENT_SPRITERENDERER_H
#define MYRIAD_CORE_COMPONENT_SPRITERENDERER_H

#include "asset/AssetManager.h"
#include "asset/Texture2D.h"
#include "core/IDrawable.h"
#include "core/component/MyrComponent.h"
#include "core/core.h"
#include "rendering/Renderer.h"
#include <string>

namespace Myriad
{
    class MYR_API SpriteRenderer : public MyrComponentBase<SpriteRenderer>,
                                   public IDrawable
    {
      protected:
        std::string texture_path_;
        TexHandle_T h_tex_;
        AssetManager *p_asset_manager_;

      public:
        SpriteRenderer(AssetManager *p_asset_manager, std::string texture_path);
        virtual ~SpriteRenderer();

        // MyrComponent
        virtual bool InitComponent(MyrObject *owner) override;
        virtual bool ReleaseComponent() override;

        // IDrawable
        virtual void Draw(Renderer &renderer);
    };
} // namespace Myriad

#endif
