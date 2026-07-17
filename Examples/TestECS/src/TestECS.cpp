#include "myriad.h"

#include "TestECS.h"

#include <cstring> //event name change

#define MOVEAMOUNT 15.0f

TestECS::~TestECS()
{
  MYR_TRACE("Destroying TestECS.");
}

void TestECS::MyCallback(Myriad::Event *e)
{
  MyEvent *me = static_cast<MyEvent *>(e);
  // This trace is slow on emscripten
  MYR_TRACE("Event callback triggered, name= %s, age= %d", me->name, me->age);
}

void TestECS::Init(Myriad::GameEngineConfig &config)
{
  config.framerate = 60;
  config.window_config.resizable = true;
  config.window_config.resolution = {800, 600};
  // fullscreen doesn't seem to matter on emscripten
  config.window_config.fullscreen = false;
  config.window_config.vsync = true;
}

void TestECS::Start()
{
  MYR_TRACE("Starting TestECS.");
  Myriad::Texture2D tex = engine_.Assets().GetTexture("shared/res/spritesheet.png");
  // MYR_TRACE("Tex is %d: %x\n", tex->GetID(), tex->GetDataPtr());
  textures[0] = tex.GetID();

  fonts[0] = engine_.Assets().GetFont("shared/res/dejavu.fnt");
  MYR_TRACE("Font is %d: %x\n", fonts[0]->GetID(), fonts[0]->GetDataPtr());

  events.Subscribe<TestECS>((Myriad::EventPrimaryType_t)1, (Myriad::EventSubType_t)1, this, &TestECS::MyCallback);

  // Set up the sprite animations
  // frames is 12 x 8
  Myriad::Vector2i fr = {12, 8};        // whole ss is 12 x 8
  Myriad::Vector2i anim_block = {3, 4}; // each animation is 3x4
  float fps = 2.0f;
  // The total num here will end up being 4 * 2 = 8
  for (int i = 0; i < (fr.x / anim_block.x) * (fr.y / anim_block.y); ++i)
  {
    // TODO this is duplicating tex info into the spritesheet.
    animations_[i] = {tex, fr, fps};

    for (int j = 0; j < anim_block.x; j++)
    {
      uint8_t col = (i % 4) * anim_block.x;     //(i % (fr.x / anim_block.x)) * (fr.x / anim_block.x);
      uint8_t row = (i / 4 % 2) * anim_block.y; //(i % (fr.y / anim_block.y)) * (fr.y / anim_block.y);
      MYR_TRACE("Anim %d: %d", i, (row * fr.x) + col);
      animations_[i].frames_up[j] = (row * fr.x) + col + j;
      animations_[i].frames_down[j] = (row * fr.x) + col + j;
      animations_[i].frames_left[j] = (row * fr.x) + col + j;
      animations_[i].frames_right[j] = (row * fr.x) + col + j;
    }
  }

  // TODO: Technically these aren't right.
  // I only want pos and movement in 'moveable'
  // and I only want pos and spritesheet in 'sprite'
  Query *q = new Query(&ecs_);
  // q->Reset().With<Position>().With<Movement>().With<SpriteAnimation>();
  q->Reset().With<Position>().With<Movement>();
  queries["moveable"] = q;

  Query *q2 = new Query(&ecs_);
  // q2->Reset().With<Position>().With<Movement>().With<SpriteAnimation>();
  q2->Reset().With<Position>().With<SpriteAnimation>();
  queries["sprite"] = q2;

  Query *q3 = new Query(&ecs_);
  q3->Reset().With<Position>().With<TestRect>();
  queries["rect"] = q3;

  movement_system_ = new MovementSystem(&ecs_, queries["moveable"]);
  render_system_ = new RenderSystem(&ecs_, queries["sprite"]);
  rect_render_system_ = new RectRenderSystem(&ecs_, queries["rect"]);

  std::string n = ecs_.GetComponentMeta<Position>().Name();
  n = "<Position>";
  std::string n2 = ecs_.GetComponentMeta<Movement>().Name();
  n2 = "<Movement>";
  std::string n3 = ecs_.GetComponentMeta<SpriteAnimation>().Name();
  n2 = "<SpriteAnimation>";

  entity_id_t rect1 = ecs_.CreateEntity();
  ecs_.AddComponent<Position>(rect1, {100.0f, 100.0f});
  ecs_.AddComponent<TestRect>(rect1, {100.0f, 50.0f, {255, 0, 0, 255}});
  ecs_.AddComponent<Movement>(rect1, {5.0f, 5.0f});

  entity_id_t rect2 = ecs_.CreateEntity();
  ecs_.AddComponent<Position>(rect2, {400.0f, 100.0f});
  ecs_.AddComponent<TestRect>(rect2, {100.0f, 50.0f, {0, 255, 0, 255}});
  ecs_.AddComponent<Movement>(rect2, {15.0f, -5.0f});
}

void TestECS::Update()
{
  ProfilerManager::Get().StartFrame();
  {
    ENGINE_PROFILE_SCOPE("Total Engine Frame");
    if (ecs_.Count() < MAX_OBJECTS)
    {
      entity_id_t eid = ecs_.CreateEntity();
      entities_.push(eid);
      ecs_.AddComponent<Position>(eid, {Myriad::MyrRandom::Float(0, engine_.GetScreenSize().x),
                                        Myriad::MyrRandom::Float(0, engine_.GetScreenSize().y)});
      ecs_.AddComponent<Movement>(eid, {Myriad::MyrRandom::Float(-MOVEAMOUNT, MOVEAMOUNT),
                                        Myriad::MyrRandom::Float(-MOVEAMOUNT, MOVEAMOUNT)});

      uint8_t anim = eid % 8;

      ecs_.AddComponent<SpriteAnimation>(eid, {anim, SpriteSheetAnimations::AnimationType::DOWN, 0.0f});
      // ecs_.AddComponent<Movement>(eid, {.pos = Myriad::Vector2(Myriad::MyrRandom::Float(0, engine_.GetScreenSize().x), Myriad::MyrRandom::Float(0, engine_.GetScreenSize().y)),
      //                                   .vel = Myriad::Vector2(Myriad::MyrRandom::Float(-MOVEAMOUNT, MOVEAMOUNT), Myriad::MyrRandom::Float(-MOVEAMOUNT, MOVEAMOUNT))});

      // MYR_TRACE("Created entity %d with Position and Movement components.", eid);
      last_id_used_ = eid;
    }
    else
    {
      size_t half = entities_.size() / 2;
      // Remove half the objects.
      for (size_t i = 0; i < half; ++i)
      {
        entity_id_t eid = entities_.front();
        entities_.pop();
        // Oh wow, AI thinks this should exist ;)
        // ecs_.DestroyEntity(eid);
        ecs_.RemoveEntity(eid);
      }
    }

    // Ideally this is event driven?
    movement_system_->Update(engine_.GetFrameElapsedMS() / 1000.0f);
  }
  ProfilerManager::Get().EndFrame();
}

void TestECS::PostUpdate()
{
  events.ProcessEvents();
}

void TestECS::Render()
{
  engine_.Renderer().ClearBackground({255, 255, 255, 255}); // white background
  // renderer_->DrawText(0, 0, "Welcome to Myriad!", {200, 200, 200, 255}); // light gray text
  // engine_.Renderer().DrawRectangle({100, 100}, {200, 150}, {255, 0, 0, 255}); // red rectangle
  // engine_.Renderer().DrawRectangle({400, 100}, {200, 150}, {0, 0, 255, 255}); // blue rectangle
  engine_.Renderer().DrawCircle({300, 400}, 50, {255, 0, 255, 255}); // green circle

  rect_render_system_->Render(engine_.GetFrameElapsedMS() / 1000.0f, engine_.Renderer());
  render_system_->Render(engine_.GetFrameElapsedMS() / 1000.0f, engine_.Renderer(), animations_);

  stats_str_.str("");
  stats_str_ << "Frame Time: " << engine_.GetFrameElapsedMS() << "ms";
  engine_.Renderer().DrawText(fonts[0], stats_str_.str(), {300, 500}, 40, {0, 0, 0, 255}); // black text
  stats_str_.str("");
  stats_str_ << "Num Objects: " << ecs_.Count() << "/ Last ID Used: " << last_id_used_;
  engine_.Renderer().DrawText(fonts[0], stats_str_.str(), {300, 530}, 20, {0, 0, 0, 255}); // black text
}
