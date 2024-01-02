#ifndef __TESTRENDERER_H_
#define __TESTRENDERER_H_

#include "myriad.h"

// #include "core/GameObject.h"
// #include "core/Transform.h"

class TestRenderer : public Myriad::Renderer
{
  public:
    TestRenderer() {}
    virtual ~TestRenderer() {}
    virtual void Draw(Myriad::TransformData t)
    {
        Myriad::Renderer::MyrDrawCircle(t.position.x, t.position.y, 10,
                                        {255, 0, 0, 255});
    }
};

class TestGameObject : public Myriad::GameObject
{
  public:
    TestGameObject() : Myriad::GameObject()
    {
        // Add the renderer
        AddComponent(&renderer);
    }
    virtual ~TestGameObject()
    {
        // Remove the renderer
        RemoveComponent(&renderer);
    }
    virtual void Draw()
    {
        Myriad::TransformData *transformData =
            (Myriad::TransformData *)m_transform.Data();
        renderer.Draw(*transformData);
    }
    virtual Myriad::Transform &GetTransform() { return m_transform; }
    virtual Myriad::Renderer &GetRenderer() { return renderer; }

  protected:
    TestRenderer renderer;
};

#endif