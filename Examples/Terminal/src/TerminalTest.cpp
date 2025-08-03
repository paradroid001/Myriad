#include "myriad_ext.h"

#include <iostream>
#include <sstream>
#include <cstring>

#include "TerminalTest.h"
#include "BasicTerminal.h"

using namespace Myriad;
using namespace Myriad::ObjectComponent;

TerminalBuffer::TerminalBuffer(uint16_t width, uint16_t height, MYR_ID_t font_id) : width(width), height(height), font(font_id)
{
  buffer = new TerminalTile *[width];
  for (int x = 0; x < width; x++)
  {
    buffer[x] = new TerminalTile[height];
    for (int y = 0; y < height; y++)
    {
      buffer[x][y].background_col = {0, 0, 0, 255};
      buffer[x][y].foreground_col = {255, 255, 255, 255};
      buffer[x][y].symbol = 'x';
    }
  }
}
TerminalBuffer::~TerminalBuffer()
{
  if (buffer != nullptr)
  {
    for (int x = 0; x < width; x++)
    {
      if (buffer[x] != nullptr)
      {
        delete[] buffer[x];
      }
    }
    delete[] buffer;
  }
};

void TerminalBuffer::Render(Renderer &renderer)
{
  Myriad::Vector2 screenpos = {0, 0};
  uint8_t size = 20;
  char str[2]; // symbol plus \0
  for (int x = 0; x < width; x++)
  {
    for (int y = 0; y < height; y++)
    {
      // print the symbol, sprintf will add the \0
      // TODO this is really inefficient
      sprintf(str, "%c", buffer[x][y].symbol);
      renderer.DrawText(font, str, {screenpos.x, screenpos.y}, size, buffer[x][y].foreground_col);
      screenpos.y += size;
    }
    screenpos.x += size;
    screenpos.y = 0;
  }
}

void TerminalBuffer::SetSymbol(uint16_t x, uint16_t y, char c)
{
  buffer[x][y].symbol = c;
}
void TerminalBuffer::SetFG(uint16_t x, uint16_t y, MyrColour fg)
{
  buffer[x][y].foreground_col = fg;
}
void TerminalBuffer::SetBG(uint16_t x, uint16_t y, MyrColour bg)
{
  buffer[x][y].background_col = bg;
}
class TerminalTest : public MyrGameApplication
{

private:
  MYR_ID_t texture1_id;
  MYR_ID_t debug_font = engine_.GetAssetManager().GetFont("res/dejavu.fnt");
  MYR_ID_t terminal_font = engine_.GetAssetManager().GetFont("res/dejavu.fnt");
  std::stringstream stats_string_;

  TerminalBuffer *term;

public:
  virtual ~TerminalTest()
  {
  }

  Myriad::EngineConfig_t Setup() override
  {
    Screen.width = 1024.0f;
    Screen.height = 768.0f;
    Myriad::EngineConfig_t config;
    config.window_title = "Test";
    config.screen_dimensions = {Screen.width, Screen.height};
    config.threads_enabled = false;
    config.num_threads = 0;
    config.fps = 60;
    config.asset_root_path = "res";
    config.max_component_slots = 100000;
    config.max_gameobject_slots = 1000;
    return config;
  }

  virtual void Start() override
  {

    // Sign up for events.
    // MyEvent::Register<MyriadExample>(1, 1, this, &MyriadExample::TestProcessMyEvent);

    debug_font = Assets().GetFont("res/dejavu.fnt");
    terminal_font = Assets().GetFont("res/dejavu.fnt");

    // Create a new terminal
    // This will get auto cleaned up when the object manager dies.
    MYR_ID_t termobj = GameObjects().CreateObject<TerminalBuffer>(20, 10, terminal_font);
    MYR_ID_t basicTermObj = GameObjects().CreateObject<BasicTerminal>(25, 40, "res/dejavu.fnt", 20);

    TerminalBuffer *term = static_cast<TerminalBuffer *>(GameObjects().GetObject(termobj));
    BasicTerminal *basicTerm = static_cast<BasicTerminal *>(GameObjects().GetObject(basicTermObj));
    basicTerm->Put("Hello, World\n");
    basicTerm->Put("More text\n");
    basicTerm->Put("A big long line of text that goes across the screen\n");
    basicTerm->Put("A big long line of text that goes across the screen\n");
    basicTerm->Put("A big long line of text that goes across the screen\n");
    basicTerm->Put("A big long line of text that goes across the screen\n");
    basicTerm->Put("A big long line of text that goes across the screen\n");
    basicTerm->Put("A big long line of text that goes across the screen\n");
    basicTerm->SetPos({100, 100});

    for (int x = 0; x < term->GetWidth(); x++)
    {
      for (int y = 0; y < term->GetHeight(); y++)
      {
        if (y < term->GetHeight() / 4)
        {
          term->SetFG(x, y, {127, 0, 127, 255});
        }
        if (y >= term->GetHeight() / 4 && y < term->GetHeight() / 2)
        {
          term->SetFG(x, y, {255, 0, 0, 255});
        }
        if (y >= term->GetHeight() / 2 && y < (3 * term->GetHeight() / 4))
        {
          term->SetFG(x, y, {255, 255, 0, 255});
        }
        if (y >= (3 * term->GetHeight() / 4))
        {
          term->SetFG(x, y, {255, 0, 255, 255});
        }
      }
    }

    term->At(4, 4).symbol = '8';
    term->At(5, 5).symbol = 'J';
    term->At(6, 6).symbol = 'G';

    term->At(0, 9).symbol = 'I';
    term->At(1, 9).symbol = 'I';
    term->At(2, 9).symbol = 'I';
    term->At(3, 9).symbol = 'I';
    term->At(4, 9).symbol = 'I';
    term->At(5, 9).symbol = 'I';
    term->At(6, 9).symbol = 'I';
    term->At(7, 9).symbol = 'I';
    term->At(8, 9).symbol = 'I';
    term->At(9, 9).symbol = 'I';

    char line[256];
    for (int y = 0; y < term->GetHeight(); y++)
    {
      memset(line, 0x0, 256);
      for (int x = 0; x < term->GetWidth(); x++)
      {
        line[x] = term->At(x, y).symbol;
      }
      MYR_INFO("{0}: {1}", y, line);
    }

    for (int x = 0; x < term->GetWidth(); x++)
    {
    }

    return; //<-- why is this line here?
  }
  void TestProcessMyEvent(Myriad::MyrEvent *p_event)
  {
    // We have to cast. This sucks.
    // MyEvent *p_myevent = static_cast<MyEvent *>(p_event);
    // MYR_INFO("Processed: {0}: {1}", p_myevent->char_data, p_myevent->num_data);
  }

  void Update() override
  {
    for (KeyCode_t k = 1; k < 336; k++) // raylib constants
    {
      if (Input.IsKeyDown(k))
      {
        TerminalKeyEvent *t = new TerminalKeyEvent();
        t->key_code = k;
        Events().AddEvent(t);
      }
    }

    for (auto go : GameObjects())
    {
      go->Update(1.0f / 60.0f);
    }

    // Process Events
    Events().ProcessEvents();
    Events().ClearEvents(); // TODO mem leak?
  }

  void Render() override
  {
    Myriad::Renderer &r = engine_.GetRenderer();
    r.BeginDrawing();
    r.ClearBackground({0, 0, 0, 255});

    /*
    for (auto p : object_manager_)
    {
      if (p != nullptr && p->IsStarted() && p->IsAlive())
      {
        p->Render(r);
      }
    }
    */
    for (auto go : GameObjects())
    {
      go->Render(r); // should render the terminal.
    }

    stats_string_.str("");
    stats_string_ << "Frame Time: " << GetFrameElapsedMS() << "ms";

    // Just draw a carrot.
    // r.DrawTexture(texture1_id, {100, 100}, {255, 255, 255, 255});

    r.DrawText(debug_font, stats_string_.str(), {20, 20}, 25, {255, 255, 255, 255});
    r.EndDrawing();
  }

  void PreShutdown() override
  {
  }
};

Myriad::MyrApplication *Myriad::CreateApplication()
{
  return new TerminalTest();
}
