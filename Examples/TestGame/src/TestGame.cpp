#include "myriad.h"

#include "TestGame.h"

#include <cstring> //event name change

#define MOVEAMOUNT 50.0f

TestGame::~TestGame()
{
  MYR_TRACE("Destroying TestGame.");
}

void TestGame::MyCallback(Myriad::Event *e)
{
  MyEvent *me = static_cast<MyEvent *>(e);
  // This trace is slow on emscripten
  MYR_TRACE("Event callback triggered, name= %s, age= %d", me->name, me->age);
}

void TestGame::Init(Myriad::GameEngineConfig &config)
{
  config.framerate = 60;
  config.window_config.resizable = true;
  config.window_config.resolution = {800, 600};
  // fullscreen doesn't seem to matter on emscripten
  config.window_config.fullscreen = false;
  config.window_config.vsync = true;
}

void TestGame::Start()
{
  MYR_TRACE("Starting TestGame.");
  Myriad::Asset *tex = engine_.Assets().GetTexture("shared/res/carrot.png");
  MYR_TRACE("Tex is %d: %x\n", tex->GetID(), tex->GetDataPtr());
  textures[0] = tex;

  fonts[0] = engine_.Assets().GetFont("shared/res/dejavu.fnt");
  MYR_TRACE("Font is %d: %x\n", fonts[0]->GetID(), fonts[0]->GetDataPtr());

  events.Subscribe<TestGame>((Myriad::EventPrimaryType_t)1, (Myriad::EventSubType_t)1, this, &TestGame::MyCallback);
}

void TestGame::Update()
{
  float dt = engine_.GetFrameElapsedMS() / 1000.0f; // convert to seconds
  Myriad::Vector2 newpos;
  Myriad::Vector2 screensize = engine_.GetScreenSize();

  for (auto it = positions.begin(); it != positions.end(); ++it)
  {
    newpos.x = it->pos.x + it->vel.x * dt;
    newpos.y = it->pos.y + it->vel.y * dt;

    if (newpos.x < 0 || newpos.x > screensize.x)
    {
      it->vel.x = -it->vel.x;
      if (newpos.x < 0)
        newpos.x = 0;
      else
        newpos.x = screensize.x;

      MyEvent *e = new MyEvent((Myriad::EventPrimaryType_t)1, (Myriad::EventSubType_t)1);
      e->age = newpos.x;
      std::strncpy(e->name, "hit side\0", sizeof(e->name));

      events.AddEvent(e);
    }
    else if (newpos.y < 0 || newpos.y > screensize.y)
    {
      it->vel.y = -it->vel.y;
      if (newpos.y < 0)
        newpos.y = 0;
      else
        newpos.y = screensize.y;

      MyEvent *e = new MyEvent((Myriad::EventPrimaryType_t)1, (Myriad::EventSubType_t)1);
      e->age = newpos.y;
      std::strncpy(e->name, "hit torb\0", sizeof(e->name));
      events.AddEvent(e);
    }
    // Actually move the item.
    // newpos has been santisized to be inside the screen.
    it->pos = newpos;
  }

  if (positions.Size() == MAX_OBJECTS)
  {
    // Remove half the objects, and start adding them back.
    for (unsigned int i = 0; i < last_id_used_; i++)
    {
      if (i % 2)
      {
        positions.Remove(i);
      }
    }
  }
  else
  {
    // Add one back at a time.
    std::size_t id = positions.Add({.pos = Myriad::Vector2(Myriad::MyrRandom::Float(0, screensize.x), Myriad::MyrRandom::Float(0, screensize.y)),
                                    .vel = Myriad::Vector2(Myriad::MyrRandom::Float(-MOVEAMOUNT, MOVEAMOUNT), Myriad::MyrRandom::Float(-MOVEAMOUNT, MOVEAMOUNT))});
    if (id == MYRIAD_INVALID_ID)
    {
      MYR_ERROR("Failed to add movement object");
    }
    else
    {
      // MYR_INFO("Added item %d", id);
      last_id_used_ = id;
    }
  }
}

void TestGame::PostUpdate()
{
  events.ProcessEvents();
}

void TestGame::Render()
{
  engine_.Renderer().ClearBackground({255, 255, 255, 255}); // white background
  // renderer_->DrawText(0, 0, "Welcome to Myriad!", {200, 200, 200, 255}); // light gray text
  engine_.Renderer().DrawRectangle({100, 100}, {200, 150}, {255, 0, 0, 255}); // red rectangle
  engine_.Renderer().DrawRectangle({400, 100}, {200, 150}, {0, 0, 255, 255}); // blue rectangle
  engine_.Renderer().DrawCircle({300, 400}, 50, {0, 255, 0, 255});            // green circle

  // ClearBackground(RAYWHITE);
  // DrawText("Welcome to Myriad!", 190, 200, 20, LIGHTGRAY);
  // DrawRectangle(100, 100, 200, 150, RED);
  // DrawRectangle(400, 100, 200, 150, BLUE);
  // DrawCircle(300, 400, 50, GREEN);

  for (auto it = positions.begin(); it != positions.end(); ++it)
  {
    engine_.Renderer().DrawTexture(textures[0], it->pos, {255, 255, 255, 255}); // white tint
    // DrawTexture(tex, positions[i].x, positions[i].y, WHITE);
  }

  stats_str_.str("");
  stats_str_ << "Frame Time: " << engine_.GetFrameElapsedMS() << "ms";
  engine_.Renderer().DrawText(fonts[0], stats_str_.str(), {300, 500}, 40, {0, 0, 0, 255}); // black text
  stats_str_.str("");
  stats_str_ << "Num Objects: " << positions.Size() << "/ Last ID Used: " << last_id_used_;
  engine_.Renderer().DrawText(fonts[0], stats_str_.str(), {300, 530}, 20, {0, 0, 0, 255}); // black text
}
