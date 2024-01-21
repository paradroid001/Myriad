#ifndef __MYRIAD_H_
#define __MYRIAD_H_

// For client applications only, not for engine to include
// #include "Entities/EntityManager.h"

// core
#include "core/Component.h"
#include "core/ComponentBase.h"
#include "core/GameObject.h"
#include "core/GroupList.h"
#include "core/IGroup.h"
#include "core/MyrApplication.h"
#include "core/Singleton.h" //in case clients want to make singletons?
#include "core/Transform.h"
#include "core/Types3D.h"
#include "core/core.h"

// audio
#include "audio/AudioManager.h"

// entities
#include "entities/GameEntity.h"
#include "entities/Updater.h"
#include "entities/UpdaterGroup.h"

// events
#include "events/Event.h"

// io
#include "io/InputManager.h"
#include "io/Json.h"
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