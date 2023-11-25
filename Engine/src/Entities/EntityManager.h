#ifndef __ENTITYMANAGER_H_
#define __ENTITYMANAGER_H_

#include "../core/core.h"
#include "flecs.h"

namespace Myriad
{
    namespace Entities
    {
        typedef flecs::world World;
        typedef flecs::entity Entity;

        class MYR_API EntityManager
        {
          public:
            virtual ~EntityManager(){};
            static EntityManager *Instance()
            {
                if (_instance == NULL)
                {
                    _instance = new EntityManager();
                }
                return _instance;
            }
            Myriad::Entities::World &World() { return *_pcurrent_world; }

          private:
            EntityManager()
            {
                // create a default world.
                _default_world = new flecs::world();
                _pcurrent_world = _default_world;
            };
            inline static Myriad::Entities::EntityManager *_instance = NULL;

          protected:
            Myriad::Entities::World *_pcurrent_world;
            Myriad::Entities::World *_default_world;
        };
    } // namespace Entities

} // namespace Myriad

#endif