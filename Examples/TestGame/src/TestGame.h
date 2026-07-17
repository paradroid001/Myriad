#include "myriad.h"

#include <sstream>

#define MAX_OBJECTS 10000

class MyEvent : public Myriad::Event
{
public:
  // TODO: So this is a major inconvenience: having to redefine the constructor.
  MyEvent(Myriad::EventPrimaryType_t p, Myriad::EventSubType_t s) : Myriad::Event(p, s) {}
  int age;
  char name[10];
};

class Movement
{
public:
  Myriad::Vector2 pos;
  Myriad::Vector2 vel;
};

class TestGame : public Myriad::MyrGameApplication
{
private:
  Myriad::Asset *textures[5];
  Myriad::Asset *fonts[5];
  Myriad::MyrIDArray<Movement, MAX_OBJECTS> positions;

  std::stringstream stats_str_;
  Myriad::MYR_ID_t last_id_used_ = MYRIAD_INVALID_ID;

  Myriad::EventSystem events;

public:
  TestGame() = default;
  virtual ~TestGame();

  void MyCallback(Myriad::Event *e);

  void Init(Myriad::GameEngineConfig &config) override;
  void Start() override;
  void Render() override;
  void Update() override;
  void PostUpdate() override;
};
