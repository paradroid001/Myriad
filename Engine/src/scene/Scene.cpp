#include "Scene.h"
#include <string>

namespace Myriad
{
    namespace Scene
    {

        Scene::Scene(const char *name) { sceneName = name; }
        Scene::~Scene(){};
        void Scene::LoadScene() {}
        void Scene::UnloadScene() {}
        void Scene::RunScene() {}
        void Scene::DrawScene() {}

        void Scene::AddGameObject(GameObjectBase &obj)
        {
            // TODO: check if it is already there
            gameObjects.push_back(&obj);
        }
        void Scene::RemoveGameObject(GameObjectBase &obj)
        {
            // TODO: what about children?
            // if (gameObjects.contains(&obj))
            //{
            gameObjects.remove(&obj);
            //}
        }
    } // namespace Scene

} // namespace Myriad
