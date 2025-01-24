#ifndef MYRIAD_MYRIAD_H
#define MYRIAD_MYRIAD_H

// Core
#include "core/IService.h"
#include "core/MyrApplication.h"
#include "core/MyrEntryPoint.h"
#include "core/core.h"

// Memory
#include "core/memory/Allocator.h"
// #include "core/memory/AllocatorProvider.h"
// #include "core/memory/AllocatorQD.h"
#include "core/memory/MyrHandle.h"

// Events
#include "core/event/MyrEvent.h"
#include "core/event/MyrEventService.h"

// Input
#include "io/KeyboardInput.h"

// Threads
#include "core/thread/IJob.h"
#include "core/thread/Job.h"
#include "core/thread/ScheduledJob.h"
#include "core/thread/ThreadPool.h"
#include "core/thread/ThreadPoolJobSystem.h"

// Assets
#include "asset/AssetManager.h"
#include "asset/AssetProvider.h"
#include "asset/Font.h"
#include "asset/MyrAsset.h"
#include "asset/Texture2D.h"
#include "asset/TextureProvider.h"

// Interfaces
#include "core/IDrawable.h"
#include "core/IEnableable.h"
#include "core/IUpdateable.h"

// Objects
#include "core/object/GameObject.h"
#include "core/object/MyrObject.h"
#include "core/object/MyrObjectManager.h"

// Components
#include "core/component/MyrComponent.h"
#include "core/component/SpriteRenderer.h"
#include "core/component/TextRenderer.h"

// IO
#include "io/Log.h"

// Rendering
#include "rendering/Renderer.h"
#include "rendering/Window.h"

#endif
