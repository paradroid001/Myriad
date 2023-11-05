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
    } // namespace Scene

} // namespace Myriad
