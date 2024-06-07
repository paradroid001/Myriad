#ifndef _MYRIAD_CORE_MYRAPPINFO_H
#define _MYRIAD_CORE_MYRAPPINFO_H
// Application information
// A data structure which holds runtime state of the game, and can be queried
// from wherever.

#include "io/Json.h"

namespace Myriad
{
    class MyrAppInfo
    {
      public:
        int device_width = 800;
        int device_height = 600;
        int render_width = 800;
        int render_height = 600;
        char *render_device;
        int log_level = 3; // error = 4, info=3, warn = 2, trace = 1
        float audio_volume = 1.0f;
        const char *resource_root_path = "res";
        const char *platform; // fill in with 'windows, linux', etc

        void FromJson(json data)
        {
            MYR_CORE_TRACE("Reading config file");
            this->device_height = data["device_height"];
            this->device_width = data["device_width"];
            this->log_level = data["log_level"];
            MYR_CORE_TRACE("End configuring application data");
        }
    };
} // namespace Myriad
#endif
