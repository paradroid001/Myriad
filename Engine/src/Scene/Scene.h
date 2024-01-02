#ifndef __SCENE_H_
#define __SCENE_H_

#include "../core/GameObject.h"
#include "../core/core.h"
// #include "core/ComponentBase.h"
#include <list>
#include <string>

namespace Myriad
{
    namespace Scene
    {
        class MYR_API Scene
        {
          public:
            Scene(const char *name);
            virtual ~Scene();
            // void OnSceneLoaded(Scene oldScene);
            // void OnSceneUnloaded(Scene newScene);
            virtual void LoadScene();
            virtual void UnloadScene();
            virtual void RunScene();
            virtual void DrawScene();
            inline std::string GetName() { return sceneName; }
            inline int GetSceneID() { return sceneId; }
            inline void SetSceneID(int id) { sceneId = id; }

            void AddGameObject(GameObjectBase &obj);
            void RemoveGameObject(GameObjectBase &obj);

          protected:
            int sceneId;
            std::string sceneName;
            std::list<GameObjectBase *> gameObjects;
        };
    } // namespace Scene

} // namespace Myriad

#endif