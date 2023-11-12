#ifndef __DOT_H_
#define __DOT_H_

// #include "core/Camera.h" //myrcolour
// #include <core/GameObject.h>
#include "core/Component.h"
#include "core/Renderer.h"
#include "core/core.h"
#include "main.h"

struct DotRendererData : Myriad::ComponentData
{
    Myriad::MyrColour colour;
    int radius;
};

class DotRenderer : public Myriad::Renderer
{
  public:
    DotRenderer() : Myriad::Renderer()
    {
        SetComponentData(&_dotRendererData);
        // RED, 3px
        SetRendererData({255, 0, 0, 255}, 3);
    };
    inline void SetRendererData(Myriad::MyrColour c, int rad)
    {
        _dotRendererData.colour = c;
        _dotRendererData.radius = rad;
    };

  protected:
    DotRendererData _dotRendererData;
};

class Dot : public Myriad::GameObject
{
  public:
    Dot();
    ~Dot();
    void Draw() override;
    void Update() override;

  private:
    Myriad::Vector3 movement;
    Myriad::MyrColour colour;
    flecs::entity e;
};

#endif