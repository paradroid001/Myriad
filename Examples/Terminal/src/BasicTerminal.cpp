#include "BasicTerminal.h"
#include "myriad.h"
using namespace Myriad;

BasicTerminal::BasicTerminal(int rows, int columns, const char *font_filename, int fontsize) : fontsize(fontsize)
{
  // font_id = e->GetAssetManager().GetFont(font_filename);
  font_id = Assets().GetFont(font_filename);
  rect.size.x = columns;
  rect.size.y = rows;
  TerminalKeyEvent::Register<BasicTerminal>(MYR_EVENT_INPUT, 0, this, &BasicTerminal::OnKeyUp);
  ClearInput();
}

BasicTerminal::~BasicTerminal()
{
  Assets().ReleaseFont(font_id);
}

bool BasicTerminal::Init()
{
  return true;
}

bool BasicTerminal::Shutdown()
{
  return true;
}

Myriad::IVector2 BasicTerminal::GetPos()
{
  return rect.pos;
}
Myriad::IVector2 BasicTerminal::SetPos(Myriad::IVector2 pos)
{
  return rect.pos = pos;
}
uint32_t BasicTerminal::Put(const char *text)
{
  std::string s(text);
  lines.push_back(s); // I guess it copies the string? copy const?
  return strlen(text);
}
void BasicTerminal::ClearScreen()
{
}
void BasicTerminal::Scroll(Myriad::IVector2 delta)
{
}

void BasicTerminal::Resize(Myriad::IVector2 newsize)
{
  rect.size = newsize;
}

void BasicTerminal::OnKeyUp(Myriad::MyrEvent *event)
{
  TerminalKeyEvent *e = static_cast<TerminalKeyEvent *>(event);
  char c = (char)e->key_code;
  if (e->key_code == 259) // raylib constant
  {
    input_line.pop_back();
  }
  else if (e->key_code == 257) // raylib constant
  {
    Put(input_line.c_str());
    ClearInput();
  }
  else
  {
    input_line += c;
  }
  // MYR_INFO("Pressed: {0}", e->key_code);
}

// void BasicTerminal::Update(float dt)
//{
//   // do nothing
// }

void BasicTerminal::Render(Myriad::Renderer &renderer)
{
  int top_offset = 0;
  int linelength = rect.size.x;
  for (long unsigned int i = 0; i < lines.size(); i++)
  {
    for (long unsigned int split = 0; split < (lines[i].length() / linelength) + 1; split++)
    {
      top_offset += fontsize;
      char c[linelength + 1];
      const char *str = lines[i].c_str();
      memset(c, 0, linelength + 1);
      strncpy(c, str + (split * linelength), linelength);
      renderer.DrawText(font_id, c, {(float)rect.Left(), (float)rect.Top() + top_offset}, fontsize, {127, 127, 127, 255});
    }
  }
  // TODO only allowing one line here for 'input' - it could be split over 2.
  top_offset += fontsize;
  renderer.DrawText(font_id, input_line, {(float)rect.Left(), (float)rect.Top() + top_offset}, fontsize, {255, 255, 255, 255});
}
