#include <cstdlib> //rand
#include <iostream>
#include <list>

#include <myriad.h>
// Entry Point
#include "Dot.h"
#include "TestEvent.h"
#include "core/Log.h"
#include <core/MyrEntryPoint.h>

#include "main.h"
// #include "Remotery.h"
#include "raylib.h"

#include "flecs.h"

Sample::Sample() {}
Sample::~Sample() {}

void Sample::Run()
{
    Myriad::Window *w = new Myriad::Window();
    MYR_TRACE("Made window class");
    w->SetFPS(60);
    MYR_TRACE("Set FPS");
    w->Init(800, 600, "Hello");
    MYR_TRACE("Inited windows");

    TestEvent *t = new TestEvent();
    t->somedata1 = 100;
    t->somedata2 = 200;

    TestEvent::Register(this, &Sample::EventHandler);

    flecs::world world;
    auto e = world.entity("Carl");
    MYR_INFO("Is {1} alive? {0}", e.is_alive(), (const char *)e.name());
    flecs::entity pos_e = world.entity<Position>();
    std::cout << "Name: " << pos_e.name()
              << std::endl; // outputs 'Name: Position'
    const EcsComponent *c = pos_e.get<flecs::Component>();
    std::cout << "Component size: " << c->size << std::endl;

    // Add a component. This creates the component in the ECS storage, but does
    // not assign it with a value.
    e.add<Velocity>();
    // Set the value for the Position & Velocity components. A component will be
    // added if the entity doesn't have it yet.
    e.set<Position>({10, 20}).set<Velocity>({1, 2});

    // Get a component
    const Position *p = e.get<Position>();
    // Remove component
    e.remove<Position>();

    // Option 2: create Tag as entity
    auto Enemy = world.entity();
    e.add(Enemy);
    e.has(Enemy); // true!

    e.remove(Enemy);
    e.has(Enemy); // false!

    // e.destruct();
    //  MYR_INFO("Is {1] alive? {0}", e.is_alive(), (const char *)e.name());

    // Relationships need to be empty structs, apparently
    struct Likes
    {
    };
    // Create a small graph with two entities that like each other
    auto Bob = world.entity("Bob");
    auto Alice = world.entity("Alice");

    Bob.add<Likes>(Alice); // Bob likes Alice
    Alice.add<Likes>(Bob); // Alice likes Bob
    Bob.has<Likes>(Alice); // true!

    Bob.remove<Likes>(Alice);
    Bob.has<Likes>(Alice); // false!

    // encode the pair (relationship description) as an id.
    flecs::id id = world.pair<Likes>(Bob);
    if (id.is_pair())
    {
        auto relationship = id.first();
        auto target = id.second();
        std::cout << "relationship=" << (const char *)relationship.name()
                  << std::endl;
        std::cout << "target=" << (const char *)target.name() << std::endl;
    }

    auto o = Alice.target<Likes>(); // Returns Bob
    std::cout << (const char *)Alice.name() << "Likes" << (const char *)o.name()
              << std::endl;

    auto e1 = world.entity().add<Position>().add<Velocity>();

    std::cout << e1.type().str() << std::endl; // output: 'Position,Velocity'
    // can also iterate
    e1.each(
        [&](flecs::id id)
        {
            if (id == world.id<Position>())
            {
                // Found Position component!
            }
        });

    // Enable the Rest API for the explorer:
    world.set<flecs::Rest>({});
    // world.app().enable_rest().run();

    // enable stats
    world.import <flecs::monitor>();

    camera = new Myriad::Camera();
    pObjects = new std::list<Myriad::GameObject *>();

    for (int i = 0; i < 10; i++)
    {
        Dot *dot = new Dot();
        Myriad::Transform *const t = dot->GetTransform();
        t->SetPosition(rand() % 800, rand() % 600, 0);

        std::cout << dot->Entity().type().str() << std::endl;

        // dot->Entity().set<Position>({t->position().x, t->position().y});

        pObjects->push_front(dot);
    }

    // System declaration

    /*
    flecs::system sys = world.system<Position, Velocity>("Move").each(
        [](Position &p, Velocity &v)
        {
            if ((p.x + v.x > 800) || (p.x + v.x < 0))
                v.x = -v.x;
            if ((p.y + v.y > 600) || (p.y + v.y < 0))
                v.y = -v.y;

            // Each is invoked for each entity
            p.x += v.x;
            p.y += v.y;
        });

    flecs::system renderdots = world.system<Position>("Render").each(
        [](const Position &p) -> void
        {
            // DrawCircleShape(p.x, p.y, 2, {100, 200, 0, 255});
            DrawCircle(p.x, p.y, 2, GREEN);
            // std::cout << "Draw" << std::endl;
        });

    */

    // Create the main instance of Remotery.
    // You need only do this once per program.
    // Remotery *rmt;
    // rmt_CreateGlobalInstance(&rmt);
    // rmt_BindOpenGL();

    int counter = 0;

    while (!w->ShouldClose())
    {
        this->Update();
        //  rmt_BeginOpenGLSample(UnscopedSample);
        this->Draw();
        //  rmt_EndOpenGLSample();
        counter += 1;
        if (counter > 60)
        {
            counter = 0;
            t->Emit();
        }

        // sys.run();
        // BeginDrawing();
        // ClearBackground({0, 0, 0, 0});
        // renderdots.run();
        // EndDrawing();
        //  stats for the ecs monitor
        //  world.progress();
    }
    w->Close();
    delete w;

    // rmt_UnbindOpenGL();
    // rmt_DestroyGlobalInstance(rmt);
    MYR_TRACE("Cleaned up and quit");
}

void Sample::Update()
{
    // rmt_BeginCPUSample(Update, RMTSF_Aggregate);
    std::list<Myriad::GameObject *>::iterator it;
    for (it = pObjects->begin(); it != pObjects->end(); ++it)
    {
        (*it)->Update();
    }
    // rmt_EndCPUSample();
}

void Sample::Draw()
{
    // rmt_BeginCPUSample(Draw, RMTSF_Aggregate);
    camera->Draw(); // pObjects);
    // rmt_EndCPUSample();
}

void Sample::EventHandler(TestEvent *e)
{
    MYR_TRACE("Event was handled! {0}, {1}", e->somedata1, e->somedata2);
}

Myriad::MyrApplication *Myriad::CreateApplication()
{
    // implementation of CreateApplication
    return new Sample();
}