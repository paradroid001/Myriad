#ifndef MYRIAD_MYRIAD_H
#define MYRIAD_MYRIAD_H

// Profiling
#include "core/MyrProfiling.h"

// Core
#include "core/core.h"
#include "core/MyrAlloc.h"
#include "core/MyrApplication.h"
#include "core/MyrGameApplication.h"
#include "core/MyrGameEngine.h"
#include "core/MyrEvent.h"
// We don't include MyrEntryPoint.h, because
// on Windows, this causes multiple definitions
// of main, despite various include guards.
// Client apps should just include myriad_ext.h

// gfx
#include "gfx/Window.h"

// Asset
#include "asset/Texture2D.h"
#include "asset/Font.h"

// #include "core/ISerialisable.h"
// #include "core/MyrEntity.h"
// #include "core/MyrEntityManager.h"
// #include "core/MyrComponent.h"

// IO
#include "io/ILogger.h"
#include "io/MyrLogging.h"
#include "io/LoggingProviderSpdLog.h"

// Util
#include "util/MyrTimer.h"

// Game
#include "game/IMyrGameObject.h"
#include "game/oc/GameObject.h"
#include "game/oc/GameObjectManager.h"
#include "game/oc/Component.h"
#include "game/oc/ComponentManager.h"
#include "game/oc/Transform.h"
#include "game/oc/SpriteRenderer.h"

#endif
