#ifndef MYRIAD_CORE_COMPONENT_SPRITERENDERER_H
#define MYRIAD_CORE_COMPONENT_SPRITERENDERER_H

#include "asset/Texture2D.h"
#include "core/IDrawable.h"
#include "core/component/MyrComponent.h"
#include "core/core.h"
#include "core/memory/Allocator.h"
#include "rendering/Renderer.h"
#include <string>

namespace Myriad
{
    class MYR_API SpriteRenderer : public MyrComponent, public IDrawable
    {
      protected:
        std::string texture_path_;
        Texture2D *p_tex_;
        Allocator *p_allocator_;

      public:
        SpriteRenderer(Allocator *p_allocator, std::string texture_path);
        virtual ~SpriteRenderer();

        // MyrComponent
        virtual bool InitComponent(MyrObject *owner) override;
        virtual bool ReleaseComponent() override;

        // IDrawable
        virtual void Draw(Renderer &renderer);
    };
} // namespace Myriad

#endif
