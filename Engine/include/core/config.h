#ifndef MYRIAD_CORE_MYRCONFIG_H
#define MYRIAD_CORE_MYRCONFIG_H

#include <cstdint>

// Myriad Renderer
#define RENDERER_NONE 0
#define RENDERER_RAYLIB 1
// 0 = none
// 1 = raylib
#define MYRIAD_RENDERER RENDERER_RAYLIB
// Max entities at once
#define MYRIAD_MAX_ENTITIES 100
// Max components at once
#define MYRIAD_MAX_COMPONENTS 100
#define MYRIAD_MAX_COMPONENTS_PER_ENTITY 16
typedef uint32_t MYR_ID_t;
#define MYRIAD_INVALID_ID 0xFFFFFFFF - 1
#define MYRIAD_ENTITY_INVALID_ID 0xFFFFFFFF - 1
#define MYRIAD_COMPONENT_INVALID_ID 0xFFFFFFFF - 1

// An event id is 16 bits wide,
// 8 bits for type, 8 for subtype.
// Changes to this would require changes in MyrEvent.h
typedef uint16_t MYREVENT_TYPE_t;
typedef uint8_t MYREVENT_PRIMARYTYPE_t;
typedef uint8_t MYREVENT_SUBTYPE_t;

namespace Myriad
{
    // 8 bits, max = 255
    typedef enum MyrEventTypes
    {
        MYR_EVENT_NONE = 0,
        MYR_EVENT_TIMER = 1,            // timers
        MYR_EVENT_SYSTEM = 2,           // close window, resize, hot reload, etc
        MYR_EVENT_INPUT = 4,            // keypress, mouse, etc
        MYR_EVENT_OBJECT = 8,           // Object destroy, creation, etc
        MYR_EVENT_FRAME_LIFECYCLE = 16, // on frameX, on renderX, on updateX
        MYR_EVENT_NET = 32,             // Network events
        MYR_EVENT_USER = 64,            // user defined events in user scripts.
        MYR_EVENT_CUSTOM = 128          // custom events (?)
    } MyrEventTypes;
} // namespace Myriad

// PROFILING
// Options:
// MYRIAD_PROFILING: OFF | ON
// MYRIAD_PROFILER: none | remotery
#define PROFILING_ON 1
#define PROFILING_OFF 0
#define PROFILER_NONE 0
#define PROFILER_REMOTERY 1
#define MYRIAD_PROFILING PROFILING_OFF
#define MYRIAD_PROFILER PROFILER_NONE

#endif
