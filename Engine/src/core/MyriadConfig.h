#ifndef __MYRIADCONFIG_H
#define __MYRIADCONFIG_H

#ifndef MYRIAD_RENDERER
    #define MYRIAD_RENDERER RAYLIB // also MYRIAD
#endif

#ifndef MYRIAD_LOGGER
    #define MYRIAD_LOGGER SPDLOG
#endif

#ifndef MYRIAD_ECS
    #define MYRIAD_ECS FLECS
#endif

/*
#define MYRIAD_PHYSICS RAYLIB  // also MYRIAD, BOX?
#define MYRIAD_AUDIO RAYLIB    // also MYRIAD
#define MRIAD_LOGGER SPDLOG    // also MYRIAD
*/

// So here is where we include the specific headers.
// We only want to do this if we are building the engine,
// i.e. when MYRIAD_INTERNAL is set.
#ifdef MYRIAD_INTERNAL

    #if MYRIAD_RENDERER == RAYLIB
        #include "raylib.h"
    #endif

    // #if MYRIAD_LOGGER == SPDLOG
    //     #include "spdlog/spdlog.h"
    // #endif

    #if MYRIAD_ECS == FLECS
        #include "flecs.h"
    #endif
#endif

#endif
