#ifndef MYRIAD_CORE_PROVIDER_H
#define MYRIAD_CORE_PROVIDER_H

namespace Myriad
{
    class Provider
    {

      public:
        virtual ~Provider();
        // virtual bool Init() = 0;
        virtual bool Shutdown() = 0;
    };
} // namespace Myriad

#endif
