#include "Transform.h"
#ifdef USE_STL_PCH
    #include "stl.h.gch"
#else
    #include <iostream>
#endif

#include "GameObject.h"
#include "core.h"
// Include the config
#include "MyriadConfig.h"

#include "Entities/EntityManager.h"

namespace Myriad
{
    GameObject::GameObject()
    {
        std::cout << "Creating GameObject" << std::endl;
        entity = Myriad::Entities::EntityManager::Instance()->World().entity();

        _internalData.gameObject = this;
        // Do we really need to add GameObjectData to the entity?
        //  entity.add<GameObjectData>();
        //  entity.set<GameObjectData>(_internalData);

        std::cout << "Gameobject is adding transform" << std::endl;
        _ptransform = new Myriad::Transform();
        AddComponent<TransformData>(_ptransform); //, _ptransform->Data());
        std::cout << "Finished adding transform" << std::endl;

        name = "DefaultName";
        // entity.set()
        std::cout << "Finished creating GameObject" << std::endl;
    }
    GameObject::~GameObject()
    {
        std::cout << "Destructor for GameObject[" << name << "]" << std::endl;
    }

    int GameObject::RemoveComponent(Myriad::Component c) { return 0; }

    Myriad::Transform *const GameObject::GetTransform()
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
} // namespace Myriad
