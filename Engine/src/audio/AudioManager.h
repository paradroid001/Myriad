#ifndef __AUDIOMANAGER_H_
#define __AUDIOMANAGER_H_

#include "core/core.h"
#include "core/Singleton.h"

namespace Myriad
{
    class MYR_API AudioManager : public Singleton<AudioManager>
    {
        public:
            void CreateChannel(const char* name);
            void SetChannelVolume(const char* name, float vol);
    };
}

#endif