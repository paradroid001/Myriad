#ifndef __SCENEMANAGER_H_
#define __SCENEMANAGER_H_

namespace Myriad
{
    namespace Scene
    {
        class Scene
        {
        };

        class SceneManager
        {
          public:
            virtual ~SceneManager(){};
            static SceneManager *Instance()
            {
                if (_instance == NULL)
                {
                    _instance = new SceneManager();
                }
                return _instance;
            }
            void RegisterScene(Scene s);
            void UnregisterScene(Scene s);
            Scene GetSceneByName(const char *name);

          private:
            SceneManager() {}
            inline static Myriad::Scene::SceneManager *_instance = NULL;
        }
    } // namespace Scene
} // namespace Myriad

#endif