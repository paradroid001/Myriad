#include "myriad.h"
#include "core/IProvider.h"
#include "ITerminal.h"

using namespace Myriad;
using namespace Myriad::ObjectComponent;

typedef struct TerminalTile
{
  MyrColour background_col;
  MyrColour foreground_col;
  char symbol;
} TerminalTile;

class TerminalBuffer : public GameObject
{
protected:
  uint16_t width;
  uint16_t height;
  TerminalTile **buffer;
  MYR_ID_t font;

public:
  virtual ~TerminalBuffer();
  TerminalBuffer(uint16_t width, uint16_t height, MYR_ID_t font_id);
  void Render(Renderer &renderer) override;

  void SetSymbol(uint16_t x, uint16_t y, char c);
  void SetFG(uint16_t x, uint16_t y, MyrColour fg);
  void SetBG(uint16_t x, uint16_t y, MyrColour bg);
  inline uint16_t GetWidth() { return width; }
  inline uint16_t GetHeight() { return height; }
  inline TerminalTile &At(uint16_t x, uint16_t y) { return buffer[x][y]; }
};
