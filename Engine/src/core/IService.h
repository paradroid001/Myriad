#ifndef MYRIAD_CORE_ISERVICE_H
#define MYRIAD_CORE_ISERVICE_H

#include "MyrHandle.h"

namespace Myriad
{
    class IService
    {
      private:
        static MyrHandle<IService> *_hinstance;
        IService()
        {
            if (_hinstance == 0)
            {
                // This should be dealloced when the last one goes out of scope.
                // TODO: Test this.
                _hinstance = new MyrHandle<IService>(this);
            }
        }; // private constructor
      public:
        virtual ~IService();
        static MyrHandle<IService> *Instance() { return _hinstance; }
        virtual bool Init() = 0;
        virtual bool Shutdown() = 0;
    };

    MyrHandle<IService> *IService::_hinstance;

    IService::~IService()
    {
        // nothing
    }

} // namespace Myriad
#endif
