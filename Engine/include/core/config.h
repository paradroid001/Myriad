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
#define MYRIAD_ENTITY_INVALID_ID 0xFFFFFFFF - 1
#define MYRIAD_COMPONENT_INVALID_ID 0xFFFFFFFF - 1

// PROFILING
// Options:
// MYRIAD_PROFILING: off = 0, on = 1
// MYRIAD_PROFILER: none = 0, remotery = 1
#define MYRIAD_PROFILER 0
#define MYRIAD_PROFILING 0

#endif
