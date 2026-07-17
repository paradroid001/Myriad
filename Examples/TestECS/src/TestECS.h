#include <sstream>

#include "myriad.h"

#include "Profiler.h"

#include "ECS2.h"
#include "ECS3.h"

#define MAX_OBJECTS 200

#define PROFILE_ENGINE 0 // Toggle to 0 in shipping builds to completely compile out overhead

#if PROFILE_ENGINE
#define ENGINE_PROFILE_SCOPE(name) ProfileScope scope##__LINE__(name)
#define ENGINE_PROFILE_FUNCTION() ENGINE_PROFILE_SCOPE(__PRETTY_FUNCTION__) // __FUNCSIG__ for MSVC. __PRETTY_FUNCTION__ is for GCC/Clang
#else
#define ENGINE_PROFILE_SCOPE(name)
#define ENGINE_PROFILE_FUNCTION()
#endif

class MyEvent : public Myriad::Event
{
public:
  // TODO: So this is a major inconvenience: having to redefine the constructor.
  MyEvent(Myriad::EventPrimaryType_t p, Myriad::EventSubType_t s) : Myriad::Event(p, s) {}
  int age;
  char name[10];
};

struct Position
{
  float x;
  float y;
};

struct Movement
{
  float x;
  float y;
};

struct TestRect
{
  float w;
  float h;
  Myriad::MyrColour color;
};

// Define animations for something.
struct SpriteSheetAnimations
{
  Myriad::Texture2D tex_asset;
  Myriad::Vector2i dim_sheet_frames; // sheet dimensions
  Myriad::Vector2i dim_frame;        // frame dimensions
  uint8_t frames_up[3];
  uint8_t frames_down[3];
  uint8_t frames_left[3];
  uint8_t frames_right[3];
  float animation_speed; // seconds

  enum class AnimationType
  {
    UP = 0,
    DOWN = 1,
    LEFT = 2,
    RIGHT = 3
  };
  SpriteSheetAnimations()
  {
  }

  SpriteSheetAnimations(Myriad::Texture2D tex, Myriad::Vector2i frames, float fps) : tex_asset(tex)
  {
    animation_speed = 1.0f / fps;
    dim_sheet_frames = frames;
    dim_frame.x = tex_asset.texture_dimensions.x / frames.x;
    dim_frame.y = tex_asset.texture_dimensions.y / frames.y;
  }
};

struct SpriteAnimation
{
  uint8_t animation_index;                             // which animation sheet?
  SpriteSheetAnimations::AnimationType animation_type; // which animation?
  float timer;
};

using namespace ECS2;

class MovementSystem : public System
{
public:
  using System::System; // inherit constructors
  void Update(float dt) override
  {
    ENGINE_PROFILE_FUNCTION(); // Automatically scopes this whole function
    // MYR_TRACE("MovementSystem::Update: dt=%f", dt);
    for (auto entity : QEntities())
    {
      auto pos = ecs_->GetComponent<Position>(entity);
      auto vel = ecs_->GetComponent<Movement>(entity);
      pos->x += vel->x * 0.1f; // dt;
      pos->y += vel->y * 0.1f; // dt;
    }
  }
};

class RectRenderSystem : public System
{
public:
  using System::System; // inherit constructors
  void Render(float dt, Myriad::IRenderer &renderer_)
  {
    ENGINE_PROFILE_SCOPE("RectRenderSystem::RectRender"); // Manual localized scope
    for (auto entity : QEntities())
    {
      auto pos = ecs_->GetComponent<Position>(entity);
      auto rect = ecs_->GetComponent<TestRect>(entity);
      renderer_.DrawRectangle({pos->x, pos->y}, {rect->w, rect->h}, rect->color);
    }
  }
};

class RenderSystem : public System
{
public:
  using System::System; // inherit constructors

  void Render(float dt, Myriad::IRenderer &renderer_, SpriteSheetAnimations *animations)
  {
    ENGINE_PROFILE_SCOPE("RenderSystem::Render"); // Manual localized scope
    // spritesheet is 12 by 8
    // float w = static_cast<Myriad::MyrTexture2D *> texture;

    // Myriad::Rect2D sprite_rect(0, 0, 64, 64); // assuming each sprite is 64x64 pixels

    for (auto entity : QEntities())
    {
      auto pos = ecs_->GetComponent<Position>(entity);
      auto spriteanim = ecs_->GetComponent<SpriteAnimation>(entity);
      // MYR_TRACE("Entity %d: Position (%f, %f)", entity, pos->x, pos->y);

      // renderer_.DrawTexture(textures[0], {pos->x, pos->y}, {255, 255, 255, 255}); // white tint
      spriteanim->timer += dt;
      SpriteSheetAnimations anim = animations[spriteanim->animation_index];
      // SpriteSheetAnimations::AnimationType anim_type = spriteanim->animation_type;

      // No actual animation - TODO: i happen to know it's three frames.
      uint8_t frame_index = 0;
      if (spriteanim->timer < anim.animation_speed)
        frame_index = 0;
      else if (spriteanim->timer < 2 * anim.animation_speed)
        frame_index = 1;
      else if (spriteanim->timer < 3 * anim.animation_speed)
      {
        frame_index = 2;
      }
      else
      {
        spriteanim->timer = 0.0f;
      }

      Myriad::Rect2D frame;
      uint8_t frame_num = anim.frames_down[frame_index];
      uint8_t col = frame_num % anim.dim_sheet_frames.x;
      uint8_t row = frame_num / anim.dim_sheet_frames.x;
      frame.pos = {col * anim.dim_frame.x * 1.0f, row * anim.dim_frame.y * 1.0f};
      frame.size = {anim.dim_frame.x * 1.0f, anim.dim_frame.y * 1.0f};
      renderer_.DrawTexture(anim.tex_asset.GetID(), frame, {pos->x, pos->y}, {255, 255, 255, 255});
    }
  }
};

class TestECS : public Myriad::MyrGameApplication
{
private:
  Myriad::AssetID_t textures[5];
  Myriad::Asset *fonts[5];
  Myriad::MyrIDArray<Movement, MAX_OBJECTS> positions;

  std::stringstream stats_str_;
  Myriad::MYR_ID_t last_id_used_ = MYRIAD_INVALID_ID;
  std::queue<entity_id_t> entities_; // so we can easily remove half
  Myriad::EventSystem events;

  ECS ecs_;
  // Query position_and_movement_query_ = Query(&ecs_);
  // Query position_and_movement_query2_ = Query(&ecs_);
  SpriteSheetAnimations animations_[8];
  std::unordered_map<std::string, Query *> queries;

  MovementSystem *movement_system_;      // = *MovementSystem(&ecs_, &position_and_movement_query_);
  RenderSystem *render_system_;          // = RenderSystem(&ecs_, &position_and_movement_query2_);
  RectRenderSystem *rect_render_system_; // = RectRenderSystem(&ecs_, &position_and_movement_query2_);

public:
  TestECS() = default;
  virtual ~TestECS();

  void MyCallback(Myriad::Event *e);

  void Init(Myriad::GameEngineConfig &config) override;
  void Start() override;
  void Render() override;
  void Update() override;
  void PostUpdate() override;
};
