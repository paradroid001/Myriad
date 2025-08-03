#ifndef ITERMINAL_H
#define ITERMINAL_H

#include "core/core.h"      //IVector2
#include "core/IProvider.h" //provider

// A terminal interface. Different terminal types can implement this interface.

namespace Myriad
{
  class ITerminal
  {
  public:
    // getcursor position
    virtual IVector2 GetPos() = 0;
    // set cursor position
    virtual IVector2 SetPos(IVector2 pos) = 0;
    // write text to cursor position
    virtual uint32_t Put(const char *text) = 0;
    virtual void ClearScreen() = 0;
    virtual void Scroll(IVector2 delta) = 0;
  };

  template <typename TerminalCellUnit>
  class TerminalProvider : public IProvider, public ITerminal
  {
  public:
    virtual bool Init() = 0;
    virtual bool Shutdown() = 0;
    virtual IVector2 GetPos() = 0;
    virtual IVector2 SetPos(IVector2 pos) = 0;
    virtual uint32_t Put(const TerminalCellUnit *text) = 0;
    virtual void ClearScreen() = 0;
    virtual void Scroll(IVector2 delta) = 0;
  };
}
#endif
