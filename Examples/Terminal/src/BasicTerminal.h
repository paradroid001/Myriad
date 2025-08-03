#ifndef BASICTERMINAL_H
#define BASICTERMINAL_H

#include <string>
#include <queue>
#include "myriad.h"      //gameobject
#include "core/config.h" //MYR_ID_t
#include "ITerminal.h"   //terminalprovider
#include "gfx/Renderer.h"

class TerminalKeyEvent : public Myriad::MyrEvent
{
public:
  // TODO just using 0 as the subtype for 'keyboard input'
  TerminalKeyEvent() : MyrEvent(Myriad::MYR_EVENT_INPUT, 0) {}
  ~TerminalKeyEvent() { MYR_INFO("Destroyed event for key {0}({1})", key_code, (char)key_code); }
  int key_code;
};

// A terminal has:
//  - A way of displaying text
//  - A way of inputting text
//  - A way of processing text to be displayed

class BasicTerminal : public Myriad::GameObject //, public Myriad::TerminalProvider<char>
{
protected:
  Myriad::IRect2D rect;
  std::deque<std::string> lines;
  MYR_ID_t font_id;
  int fontsize;
  std::string input_line;

public:
  BasicTerminal(int rows, int columns, const char *font_filename, int fontsize);
  ~BasicTerminal();

  // TerminalProvider
  virtual bool Init();     // override;
  virtual bool Shutdown(); // override;
  // getcursor position
  virtual Myriad::IVector2 GetPos(); // override;
  // set cursor position
  virtual Myriad::IVector2 SetPos(Myriad::IVector2 pos); // override;
  // write text to cursor position
  virtual uint32_t Put(const char *text);      // override;
  virtual void ClearScreen();                  // override;
  virtual void Scroll(Myriad::IVector2 delta); // override;

  // Not in the interface yet
  virtual void Resize(Myriad::IVector2 newsize);
  void OnKeyUp(Myriad::MyrEvent *event);
  void ClearInput() { input_line = ":>"; }

  // GameObject
  // virtual void Update(float dt) override;
  void Render(Myriad::Renderer &renderer) override;
};

#endif
