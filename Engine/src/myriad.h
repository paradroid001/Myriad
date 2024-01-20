#ifndef __MYRIAD_H_
#define __MYRIAD_H_

// For client applications only, not for engine to include
// #include "Entities/EntityManager.h"

// core
#include "core/Component.h"
#include "core/ComponentBase.h"
#include "core/GameObject.h"
#include "core/MyrApplication.h"
#include "core/Transform.h"
#include "core/Types3D.h"
#include "core/core.h"
#include "core/IGroup.h"
#include "core/GroupList.h"


// entities
#include "entities/GameEntity.h"
#include "entities/Updater.h"
#include "entities/UpdaterGroup.h"

// events
#include "events/Event.h"

// io
#include "io/Log.h"

// rendering
#include "rendering/Camera.h"
#include "rendering/Renderer.h"
#include "rendering/RendererGroup.h"
#include "rendering/Window.h"

// scene
#include "scene/Scene.h"
#include "scene/SceneManager.h"

// --- Entry Point ---
// #include "core/MyrEntryPoint.h"
// -------------------

// STD sfuff
// #include "stdio.h"

#endif