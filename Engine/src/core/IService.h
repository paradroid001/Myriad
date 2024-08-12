#ifndef MYRIAD_CORE_ISERVICE_H
#define MYRIAD_CORE_ISERVICE_H

// #include "core/memory/MyrHandle.h"
#include "core/MyrSingleton.h"

namespace Myriad
{
    this is a note to tell myself where I am up to.
    cant have an abstract class as a singleton, that wont work,
    would have to make some service inherit from IService and from MyrSingleton.

    class IService : public MyrSingleton<IService>
    {
        // This using line
        using MyrSingleton<IService>::MyrSingleton;
        /*
        private:
          static IService *_hinstance;
          IService()
          {
              if (_hinstance == 0)
              {
                  // This should be dealloced when the last one goes out of
        scope.
                  // TODO: Test this.
                  _hinstance = this;
              }
          } // private constructor
        */
      protected:
        bool started_;
        virtual bool WhenStartService() = 0;
        virtual bool WhenStopService() = 0;

      public:
        virtual ~IService() = 0;
        // static IService *Instance() { return _hinstance; }
        bool StartService()
        {
            bool ret = WhenStartService();
            started_ = true;
            return ret;
        }
        bool StopService()
        {
            bool ret = WhenStopService();
            started_ = false;
            return ret;
        }
    };

    // MyrHandle<IService> *IService::_hinstance;

    /*
    IService::~IService()
    {
        // nothing
    }*/

} // namespace Myriad
#endif
