#ifndef MYRIAD_GAME_GAMEOBJECT_SPRITERENDEERER
#define MYRIAD_GAME_GAMEOBJECT_SPRITERENDEERER

#include "asset/AssetManager.h"
#include "asset/Texture2D.h"
#include "core/core.h"
#include "game/oc/Component.h"
#include "gfx/Renderer.h"

namespace Myriad::ObjectComponent
{
    using namespace Myriad;
    class MYR_API SpriteRenderer : public Component<SpriteRenderer>
    {
      protected:
        TexHandle_T texid_;
        AssetManager *p_asset_manager_;
        bool alloced_; // did we alloc this texture, or were we passed it?
        Vector2 offset_;
        Vector2 size_;

      public:
        void Init(AssetManager *asset_manager, std::string &path);
        // Constructor which sets the right component type.
        // See Component.h
        // SpriteRenderer() : Component(COMPONENT_TYPE_SPRITERENDERER) {}
        virtual ~SpriteRenderer();

        void Update(float dt);
        void Render(Renderer &renderer);
    };
} // namespace Myriad::ObjectComponent

#endif
