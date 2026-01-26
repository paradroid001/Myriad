#include "myriad_ext.h"

#include "Events.h"

typedef enum
{
  DAMAGE = 12,
  END_OF_TURN = 13
} EventTypes_t;

class DamageEvent : public events::Event<EventTypes_t>
{
public:
  int amount;
  DamageEvent(events::EventSystem *es, int val) : Event<EventTypes_t>(es), amount(val)
  {
    event_type = EventTypes_t::DAMAGE;
  }
  virtual ~DamageEvent() {}
};

class EndOfTurnEvent : public events::Event<EventTypes_t>
{
public:
  int turn_number;
  EndOfTurnEvent(events::EventSystem *es, int val) : Event<EventTypes_t>(es), turn_number(val)
  {
    event_type = EventTypes_t::END_OF_TURN;
  }
  virtual ~EndOfTurnEvent() {}
};

class TestEventGame : public MyrGameApplication
{
public:
  ~TestEventGame() {}
  Myriad::EngineConfig_t Setup() override
  {
    Screen.width = 800.0f;
    Screen.height = 600.0f;
    Myriad::EngineConfig_t config;
    config.window_title = "Event Test";
    config.screen_dimensions = {Screen.width, Screen.height};
    config.threads_enabled = false;
    config.num_threads = 0;
    config.fps = 60;
    config.asset_root_path = "res";
    config.max_component_slots = 100000;
    config.max_gameobject_slots = 1000;
    return config;
  }

  void HandleDamage(events::IEvent *e)
  {
    DamageEvent *d = static_cast<DamageEvent *>(e);
    MYR_TRACE("Damage Event is handled, amount is {0}", d->amount);
  }

  void HandleEndOfTurn()
  {
    MYR_TRACE("End of turn handled");
  }

  virtual void Start() override
  {
    // So this is just a test with no event system, only a dispatcher.
    // It is far from what we want.
    // So many arguments to Subscribe!
    // Ideally subscribe would be Subscribe<DamageEvent>(instance, function);
    // Ideally Emit would be e.Emit(); (it already knows itself..)
    events::EventDispatcher dispatcher;
    dispatcher.Subscribe<DamageEvent, TestEventGame>(DAMAGE, this, &TestEventGame::HandleDamage);
    DamageEvent e(nullptr, 3);
    e.Emit(dispatcher);
  }

  void Update() override
  {
  }
  void Render() override
  {
    Myriad::Renderer &r = engine_.GetRenderer();
    r.BeginDrawing();
    r.ClearBackground({127, 127, 127, 255});
    r.EndDrawing();
  }
};

Myriad::MyrApplication *Myriad::CreateApplication()
{
  return new TestEventGame();
}
