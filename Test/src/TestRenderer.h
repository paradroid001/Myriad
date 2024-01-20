#ifndef __TESTRENDERER_H_
#define __TESTRENDERER_H_

#include "myriad.h"
#include <cstdlib>

// #include "core/GameObject.h"
// #include "core/Transform.h"

class TestRenderer : public Myriad::Renderer
{
  public:
    TestRenderer() : Myriad::Renderer() {}
    virtual ~TestRenderer() {}
    virtual void Draw(Myriad::TransformData t)
    {
        Myriad::Renderer::MyrDrawCircle(t.position.x, t.position.y, 10,
                                        {255, 0, 0, 255});
    }
};

struct TestUpdateData: public Myriad::ComponentData
{
  public:
    Myriad::Vector2 velocity;
    Myriad::TransformData *p_transform;
};

class TestUpdater : public Myriad::Updater
{
  public:
    TestUpdater() : Myriad::Updater()
    {
      SetComponentData(&m_updateData);
      m_updateData.velocity.x = rand() % 6 + 3;
      m_updateData.velocity.y = rand() % 6 + 3;
    }
    virtual ~TestUpdater(){}
    inline virtual void Update(float dt) override
    {
      int x= m_updateData.p_transform->position.x;
      int y= m_updateData.p_transform->position.y;

      Myriad::Vector2 newp = {x + m_updateData.velocity.x, y + m_updateData.velocity.y};
      if (newp.x < 0 || newp.x > 800)
      {
        m_updateData.velocity.x = -m_updateData.velocity.x;
      }
      if (newp.y < 0 || newp.y > 600)
      {
        m_updateData.velocity.y = -m_updateData.velocity.y;
      }

      

      m_updateData.p_transform->position.x += m_updateData.velocity.x;
      m_updateData.p_transform->position.y += m_updateData.velocity.y;
      
    }
  protected:
    TestUpdateData m_updateData;
};

class TestGameObject : public Myriad::GameObject
{
  public:
    TestGameObject() : Myriad::GameObject()
    {
        //Set the renderer up with the transform of this gameobject
        Myriad::RendererData *rd = (Myriad::RendererData*)renderer.Data();
        rd->p_transformData = (Myriad::TransformData *)m_transform.Data();
        
        // Add the renderer
        AddComponent(&renderer);

        TestUpdateData *ud = (TestUpdateData*)updater.Data();
        ud->p_transform = rd->p_transformData;
        AddComponent(&updater);
    }
    virtual ~TestGameObject()
    {
        //let the gameobject destructor do this.
        // Remove the renderer
        //RemoveComponent(&renderer);
    }
    virtual void Draw()
    {
        Myriad::TransformData *transformData =
            (Myriad::TransformData *)m_transform.Data();
        renderer.Draw(*transformData);
    }
    
    inline virtual Myriad::Transform & GetTransform() { return m_transform; }
    inline virtual Myriad::Transform const & GetTransform() const { return m_transform; }
    inline virtual Myriad::Transform * GetTransformP() {return &m_transform; }
    virtual Myriad::Renderer &GetRenderer() { return renderer; }
    virtual Myriad::Updater &GetUpdater() { return updater; }

  protected:
    TestRenderer renderer;
    TestUpdater updater;
};

#endif