#include "ComponentBase.h"
#include "Transform.h"
#include <iostream>

#include "GameObject.h"
#include "Log.h"
#include "core.h"
// Include the config
#include "MyriadConfig.h"

// #include "Entities/EntityManager.h"

namespace Myriad
{
    GameObject::GameObject()
    {
        std::cout << "Creating GameObject" << std::endl;
        // entity =
        // Myriad::Entities::EntityManager::Instance()->World().entity();

        //_internalData.gameObject = this;
        // Do we really need to add GameObjectData to the entity?
        //  entity.add<GameObjectData>();
        //  entity.set<GameObjectData>(_internalData);

        MYR_CORE_TRACE("Gameobject is adding transform");
        _ptransform = new Myriad::Transform();
        // AddComponent<TransformData>(_ptransform); //, _ptransform->Data());
        MYR_CORE_TRACE("Finished adding transform, but didn't actually add it");

        name = "DefaultName";
        // entity.set()
        MYR_CORE_TRACE("Finished creating GameObject");
    }
    GameObject::~GameObject()
    {
        MYR_CORE_TRACE("Destructor for GameObject[{0}]", name);
    }

    void GameObject::AddComponent(Myriad::ComponentBase *c)
    {
        MYR_CORE_TRACE("GameObject::AddComponent is not implemented.");
    }

    void GameObject::RemoveComponent(Myriad::ComponentBase *c)
    {
        MYR_CORE_TRACE("GameObject::RemoveComponent is not implemented.");
    }

    int GameObject::GetNumChildren() { return 0; }

    GameObjectBase **GameObject::GetChildren()
    {
        MYR_CORE_TRACE("GameObject::GetChildren not implemented");
        return 0;
    }
    GameObjectBase *GameObject::GetChild(int i)
    {
        MYR_CORE_TRACE("GameObject::GetChild not implemented");
        return (GameObjectBase *)this;
    }

    int GameObject::AddChild(GameObjectBase *child)
    {
        MYR_CORE_TRACE("GameObject::AddChild not implemented");
        return 0;
    }

    int GameObject::RemoveChild(GameObjectBase *child)
    {
        MYR_CORE_TRACE("GameObject::RemoveChild not implemented");
        return 0;
    }

    int GameObject::RemoveChildn(int n)
    {
        MYR_CORE_TRACE("GameObject::RemoveChildn not implemented");
        return 0;
    }

    void GameObject::Enable()
    {
        MYR_CORE_TRACE("GameObject::Enable not implemented");
    }

    void GameObject::OnEnable()
    {
        MYR_CORE_TRACE("GameObject::OnEnable not implemented");
    }

    void GameObject::Disable()
    {
        MYR_CORE_TRACE("GameObject::Disable not implemented");
    }

    void GameObject::OnDisable()
    {
        MYR_CORE_TRACE("GameObject::OnDisable not implemented");
    }

    /*
    int GameObject::

        Myriad::Transform *const
        GameObject::GetTransform()
    {
        return (Transform *const)_ptransform;
    }

    std::string GameObject::GetName() { return name; }

    void GameObject::SetName(const char *newname)
    {
        name = "This is hardcoded!";
    }

    void GameObject::Update()
    {
        //   override me
    }

    void GameObject::Draw()
    {
        Color c = GREEN;
        MyrColour mc;
        mc.r = c.r;
        mc.g = c.g;
        mc.b = c.b;
        mc.a = c.a;
        // override me
        this->DrawCircleShape(_ptransform->position().x,
                              _ptransform->position().y, 20, mc);
    }

    void GameObject::DrawCircleShape(float x, float y, float r, MyrColour c)
    {
#if MYRIAD_RENDERER == RAYLIB
        DrawCircle(x, y, r, {c.r, c.g, c.b, c.a});
#endif
    }
    */
} // namespace Myriad
