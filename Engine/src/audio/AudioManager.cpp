#include "audio/AudioManager.h"
#include "io/Log.h"

namespace Myriad
{
    void AudioManager::CreateChannel(const char* name)
    {
        MYR_CORE_TRACE("Creating audio channel {0}", name);
    }
    void AudioManager::SetChannelVolume(const char* name, float vol)
    {
        MYR_CORE_TRACE("Setting volume for {0} to {1}", name, vol);
    }
}